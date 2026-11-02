# Notice : comment Organic Maps calcule un itinéraire

Diagramme associé : [calcul-itineraire.svg](calcul-itineraire.svg)

Tout le calcul est **local** (aucun serveur) : il utilise uniquement les fichiers `.mwm` téléchargés.
Le cœur se trouve dans `libs/routing/` (algorithme) et `libs/routing_common/` (modèles de véhicules).

---

## 1. Vue d'ensemble du pipeline

| Étape | Rôle | Fichier principal |
|---|---|---|
| 1. Demande | L'UI pose départ / étapes / arrivée, choisit le type de véhicule | `libs/map/routing_manager.cpp` |
| 2. Thread de calcul | Le calcul tourne en arrière-plan, annulable | `libs/routing/async_router.cpp` |
| 3. Chargement des options | Lecture des routes à éviter (péage, autoroute, ferry, chemins, escaliers) | `routing_options.cpp` → `IndexRouter::CalculateRoute` |
| 4. Recalage (snapping) | Projection des points sur les routes les plus proches, en tenant compte de la direction (voiture) et en écartant les impasses | `IndexRouter::PointsOnEdgesSnapping` |
| 5. Choix du mode de graphe | Voiture longue distance = « leaps » (sauts entre cartes), sinon graphe complet | `IndexRouter::SetupAlgorithmMode` |
| 6. Recherche A* bidirectionnelle | Trouve le chemin de poids minimal | `libs/routing/base/astar_algorithm.hpp` |
| 7. Coût de chaque tronçon | Vitesse × facteurs × trafic × pente → secondes | `edge_estimator.cpp`, `routing_common/vehicle_model.cpp` |
| 8. Pénalités de transition | Demi-tour, embarquement ferry, accès privé, transit, restrictions de tourne | `index_graph.cpp`, `route_weight.cpp` |
| 9. Itinéraire alternatif | 2ᵉ calcul avec l'autre stratégie (« plus court ») | `IndexRouter::CalculateRoute` |
| 10. Reconstruction | Géométrie, noms de rues, radars, durée (ETA) | `IndexRouter::RedressRoute` |
| 11. Instructions | Virages, voies, annonces vocales | `turns_generator.cpp`, `car_directions.cpp` |
| 12. Suivi / recalcul | Détection de sortie d'itinéraire, ajustement ou recalcul complet | `routing_session.cpp` |

---

## 2. Le « poids » : ce que l'algorithme minimise

L'A* ne minimise **pas** la distance mais un **poids exprimé en secondes** (`RouteWeight`).
Il y a deux calculs distincts pour chaque tronçon (`EdgeEstimator::Purpose`) :

- **Weight** : sert à *choisir* le chemin (peut contenir des pénalités artificielles) ;
- **ETA** : sert à *afficher* la durée estimée à l'utilisateur.

### 2.1 Vitesse d'un tronçon (voiture)

Calculée dans `VehicleModel::GetTypeSpeedImpl` (`libs/routing_common/vehicle_model.cpp`) :

1. Si OSM fournit un `maxspeed` → on part de cette vitesse.
2. Sinon → vitesse par défaut selon le type de route et « en ville / hors ville »
   (`kHighwayBasedSpeeds` dans `libs/routing_common/car_model_coefs.hpp`).
   Exemples (km/h, ville / hors ville) : motorway 118/124, trunk 90/103, primary 65/82,
   secondary 60/70, tertiary 50/50, residential 20-26, service 15, track 5.
3. Multipliée par un **facteur par type de route** (`kHighwayBasedFactors`) :
   motorway 0,95, primary 0,85, tertiary 0,70-0,75, residential 0,70…
4. Multipliée par un **facteur de revêtement** (`kCarSurface` dans `car_model.cpp`) :
   bon goudron 1,0 ; mauvais goudron 0,6 ; non revêtu bon 0,4 ; non revêtu mauvais 0,2.

Puis dans `CarEstimator::CalcSegmentWeight` (`libs/routing/edge_estimator.cpp`) :

```
poids = distance / vitesse
      × facteur trafic (si données trafic ; ×1,8 si embouteillage connu ; ×10 000 si route bloquée)
```

Piéton et vélo ajoutent une **pénalité de pente** (`GetPedestrianClimbPenalty`, `GetBicycleClimbPenalty`).
Pour la voiture la pente est ignorée (`GetCarClimbPenalty` renvoie 1).

### 2.2 Pénalités ajoutées aux transitions (`IndexGraph::CalculateEdgeWeight`)

| Pénalité | Valeur voiture | Où la changer |
|---|---|---|
| Demi-tour | 120 s | `CarEstimator::GetUTurnPenalty` |
| Embarquement ferry | 20 min | `CarEstimator::GetFerryLandingPenalty` |
| Traverser une zone « no-through » (résidentiel/service fermé au transit) | 30 min par changement | `RouteWeight::s_PassThroughPenaltyS` (`route_weight.cpp`) |
| Entrer/sortir d'un accès privé/destination | 2 h par changement | `RouteWeight::s_AccessPenaltyS` |
| Accès conditionnel (horaires OSM) | voir `GetIntegratedWeight` | `route_weight.cpp` |
| Interdictions de tourner OSM (`restriction=*`) | arête supprimée | `restriction_loader.cpp`, `index_graph.cpp` |

Valeurs vélo / piéton : demi-tour 20 s / 0 s, ferry 10 min (piéton).

### 2.3 Options « éviter » (bloquantes, pas des pénalités)

`RoutingOptions::Road` (`libs/routing/routing_options.hpp`) : `Toll`, `Motorway`, `Ferry`, `Dirty`, `Steps`.
Une route marquée par une option activée est **retirée du graphe** (`RoadGeometry::SuitableForOptions`).
Correspondance tag OSM → option dans `RoutingOptionsClassifier` (`routing_options.cpp`) :
`highway=motorway` → Motorway, `hwtag=toll` → Toll, `route=ferry` → Ferry, `highway=track` → Dirty.

Si aucune route n'est trouvée avec ces exclusions, le calcul échoue (pas de repli automatique).

---

## 3. L'itinéraire alternatif (« plus rapide » vs « plus court »)

`EdgeEstimator::Strategy` :

- **Normal** : modèle complet (vitesse, trafic, pénalités).
- **DistanceBiased** : toute route dont la vitesse ≥ un plafond est comptée *à la distance* (poids = distance / plafond).
  Les routes lentes gardent leur poids normal → on obtient le chemin court mais « raisonnable »
  (pas de chemins de terre ni de ruelles).
  Plafond : voiture **40 km/h**, vélo 12 km/h, piéton 4 km/h (`kDistanceBiasCapSpeedKMpH`).

Règles (`IndexRouter::CalculateRoute`, `index_router.cpp`) :

- calculé seulement si départ–arrivée ≤ **300 km** (voiture) ou 100 km (autres) ;
- montré seulement si son ETA ≤ **1,5 ×** celle de la route principale (`kMaxAltEtaRatio`) ;
- choisir l'alternative bascule la stratégie active (`SwapAltRouteToActive`), conservée pour les recalculs.

---

## 4. Modes de graphe (performance)

`IndexRouter::SetupAlgorithmMode` :

- **Voiture, cartes éloignées (> 300 km, `kCloseMwmPointsDistanceM`)** : `LeapsOnly` — on saute de frontière en frontière
  entre cartes grâce à des poids **pré-calculés au moment de la génération des .mwm** (section cross-mwm),
  puis chaque saut est « déplié » en vraies routes (`ProcessLeapsJoints`).
- **Voiture proche, vélo, piéton** : `Joints` (graphe compressé aux intersections).
- **Transports en commun / traces guides** : `NoLeaps`.

Conséquence importante : modifier les vitesses dans le code change le choix *à l'intérieur* des cartes, mais les
poids de saut entre cartes restent ceux calculés par le générateur → pour les longs trajets il faut aussi
régénérer les cartes pour un effet complet.

---

## 5. Recalcul en navigation

`RoutingSession::OnLocationPositionChanged` (`routing_session.cpp`) :

- un compteur augmente quand on s'éloigne de l'itinéraire (+1 lent, +2 si vitesse ≥ 3 km/h) ;
  au-delà de **10** (`kOnRouteMissedCount`) → état `RouteNeedRebuild` ;
- **ajustement** (raccord rapide sur l'ancien itinéraire) si on est à ≤ **5 km** de la route et à ≥ **10 km**
  de l'arrivée (`kAdjustRangeM`, `kMinDistanceToFinishM`) ; sinon recalcul complet ;
- changer une option « éviter » invalide le cache et force un recalcul complet.

Paramètres par véhicule dans `GetRoutingSettings` (`routing_settings.cpp`) : seuil de recalage (voiture 50 m),
tolérance d'arrivée (20 m), utilisation de la direction de déplacement pour le départ, etc.

---

## 6. Ce que tu peux changer — récapitulatif

### Sans toucher au code (réglages utilisateur)

| Réglage | Effet | Stockage |
|---|---|---|
| Éviter péages / autoroutes / ferries / chemins non revêtus | Retire ces routes du graphe | `settings` clé par véhicule (`RoutingOptions::SaveToSettings`) |
| Optimisation de l'ordre des étapes | Réordonne les points intermédiaires ajoutés | clé `RouteOptimizationEnabled` (`DrivingOptionsFragment.java`) |
| Sélection de l'alternative | Passe en mode « plus court » pour la suite | `IndexRouter::SwapAltRouteToActive` |

### En modifiant le code (fork)

| Je veux… | Modifier | Fichier |
|---|---|---|
| Préférer / fuir certaines catégories de routes | `kHighwayBasedFactors` (poids, sans changer l'ETA si tu touches seulement `weight`) | `libs/routing_common/car_model_coefs.hpp` |
| Changer la vitesse supposée par type de route | `kHighwayBasedSpeeds` | idem |
| Fuir les routes en mauvais état | `kCarSurface` | `libs/routing_common/car_model.cpp` |
| Moins / plus de demi-tours | `CarEstimator::GetUTurnPenalty` | `libs/routing/edge_estimator.cpp` |
| Éviter davantage les ferries | `GetFerryLandingPenalty` | idem |
| Ajuster « plus court » | `kDistanceBiasCapSpeedKMpH` du `CarEstimator` | idem |
| Toujours/jamais proposer l'alternative | `kMaxAltEtaRatio`, `altMaxDistanceM` | `libs/routing/index_router.cpp` |
| Éviter les traversées de quartiers / accès privés | `s_PassThroughPenaltyS`, `s_AccessPenaltyS` | `libs/routing/route_weight.cpp` |
| Ajouter une nouvelle option « éviter » | enum `Road` + `RoutingOptionsClassifier` + `GetSupportedOptions` + UI | `libs/routing/routing_options.*`, Android `DrivingOptionsFragment` |
| Pénaliser (au lieu d'exclure) une option | ajouter un facteur dans `CarEstimator::CalcSegmentWeight` selon `road.GetRoutingOptions()` | `edge_estimator.cpp` |
| Recalcul plus/moins réactif | `kOnRouteMissedCount`, `m_matchingThresholdM` | `routing_session.cpp`, `routing_settings.cpp` |
| Ajustement vs recalcul complet | `kAdjustRangeM`, `kMinDistanceToFinishM` | `index_router.cpp` |

### Précautions

- Ne modifier que la composante **`weight`** d'un `SpeedFactor` change le choix d'itinéraire sans fausser l'ETA affiché.
- L'heuristique A* utilise la vitesse max du modèle (`CalcHeuristic`) : ne jamais rendre un poids
  *inférieur* à `distance / vitesse max`, sinon l'A* n'est plus optimal.
- Les tests d'intégration (`libs/routing/routing_integration_tests/`, ex. `*_KeepMotorway`) vérifient ces
  coefficients : à relancer après chaque réglage (nécessitent des cartes).
- Les poids cross-mwm (long trajets voiture) sont calculés par le générateur (`generator/`) : régénérer les
  `.mwm` pour que les nouveaux coefficients s'appliquent aussi aux sauts entre cartes.
