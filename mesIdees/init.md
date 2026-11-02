Voici un prompt destiné à OpenAI Codex pour modifier un fork d'Organic Maps sur Android.

L'objectif est d'intégrer un système d'alertes de zones de danger françaises, entièrement hors ligne, directement dans Organic Maps, sans application compagnon ni service externe.

Le prompt impose à Codex d'étudier d'abord le code existant, notamment la gestion des radars, afin de réutiliser les fonctionnalités déjà présentes et d'éviter une réécriture inutile.

# Mission Codex : intégrer des alertes de zones de danger dans Organic Maps Android

## Contexte

Je souhaite développer un fork personnel du projet open source Organic Maps.

Dépôt officiel : [https://github.com/organicmaps/organicmaps](https://github.com/organicmaps/organicmaps)

Plateforme cible : Android.

Environnement de développement : Windows 11, Android Studio, Java, Kotlin, C++ et CMake selon les composants existants.

**Objectif principal :** intégrer nativement un système de signalement des zones de danger françaises, fonctionnant entièrement hors ligne, sans publicité, sans abonnement et sans serveur externe.

L'application doit conserver l'expérience originale d'Organic Maps : navigation GPS, cartographie épurée, respect de la vie privée et calcul local des itinéraires.

## Phase 1 — Analyse obligatoire du code existant

Avant toute modification :

1. Examiner l'architecture du projet, notamment les modules Android, C++, navigation, géolocalisation et synthèse vocale.
2. Identifier l'implémentation existante des alertes de radars et des avertissements de navigation.
3. Rechercher les mécanismes associés à `speed_camera`, `SpeedCamera`, `speedcams`, ainsi qu'à la commande de débogage `?debug-cam`.
4. Déterminer comment les données de radars sont actuellement stockées et extraites des cartes OpenStreetMap.
5. Identifier les éventuelles restrictions géographiques et les mécanismes de configuration.
6. Examiner les possibilités de réutiliser la synthèse vocale et les notifications existantes.

**Impératif : privilégier la réutilisation du code existant plutôt que développer un second moteur d'alertes indépendant.**

Produire un court compte rendu de cette analyse dans `docs/ZONE_DANGER_ANALYSIS.md`, avec les principaux fichiers et composants concernés.

## Phase 2 — Gestion des zones de danger

Développer une fonctionnalité nommée « Zones de danger ».

Exigences :

- Compatible avec la réglementation française.
- Ne pas afficher les positions exactes des radars ou contrôles routiers.
- Représenter les zones de danger sous forme de segments routiers, avec un début et une fin.
- Tenir compte du sens de circulation.
- Fonctionner sans connexion Internet.
- Utiliser les données déjà disponibles dans Organic Maps lorsque cela est pertinent et légalement exploitable.
- Prévoir l'importation optionnelle d'une base de zones de danger au format GeoJSON ou d'un format local adapté.
- Permettre de remplacer ou actualiser manuellement cette base.
- Stocker les données localement, sans synchronisation réseau obligatoire.

Pour la France, prévoir une longueur de zone configurable dans les limites réglementaires applicables et rechercher les règles exactes avant implémentation.

Ne pas assimiler automatiquement chaque position de radar OSM à un point d'alerte affichable.

## Phase 3 — Détection des zones de danger

Le système doit exploiter la position GPS et, si disponible, l'itinéraire actuellement suivi.

Comportement attendu :

- Détecter les zones situées devant le véhicule.
- Ignorer les zones derrière le véhicule.
- Ignorer les zones correspondant à une route parallèle ou à un sens de circulation opposé.
- Privilégier la correspondance avec le trajet calculé.
- Sans itinéraire actif, utiliser une correspondance avec la route suivie, en tenant compte de la précision GPS et du cap.
- Éviter les déclenchements multiples pour une même zone.
- Réinitialiser correctement les alertes lorsqu'une zone est quittée ou lorsqu'un nouvel itinéraire est calculé.

Implémenter une machine à états simple :

`OUTSIDE → APPROACHING → INSIDE → EXITED`

Prévoir des seuils configurables et une hystérésis pour éviter les déclenchements intempestifs dus aux imprécisions GPS.

## Phase 4 — Alertes vocales

Réutiliser en priorité le moteur d'annonces vocales déjà présent dans Organic Maps.

Prévoir :

- Une annonce à l'approche : « Attention, zone de danger à venir. »
- Une indication visuelle discrète pendant la traversée.
- Une notification optionnelle de sortie de zone.
- Une option pour désactiver les alertes vocales.
- Un réglage du délai d'anticipation.
- L'absence de répétition excessive.
- La compatibilité avec les annonces de navigation existantes.

Les alertes ne doivent pas interrompre abusivement les instructions de guidage.

## Phase 5 — Interface Android

Ajouter une section dans les paramètres de navigation :

**Navigation → Zones de danger**

Options souhaitées :

- Activer les alertes de zones de danger.
- Activer les alertes vocales.
- Afficher ou masquer les zones sur la carte.
- Choisir le niveau d'anticipation : court, normal, long.
- Activer ou désactiver les avertissements de sortie de zone.
- Importer une base de zones de danger.
- Afficher la date et la provenance de la base installée.

L'interface doit respecter l'architecture, les composants graphiques, les traductions et le style d'Organic Maps.

## Phase 6 — Performances et confidentialité

Contraintes obligatoires :

- Aucun service cloud.
- Aucune télémétrie supplémentaire.
- Aucun SDK publicitaire.
- Aucun suivi des déplacements.
- Aucune requête réseau pour déclencher les alertes.
- Ne pas réintroduire des dépendances Google Play Services.
- Consommation CPU et batterie limitée.
- Utiliser un index spatial local plutôt qu'effectuer une recherche exhaustive à chaque position GPS.
- Ne pas créer de service Android permanent supplémentaire si le fonctionnement existant d'Organic Maps suffit.

## Phase 7 — Tests

Ajouter des tests unitaires et, lorsque pertinent, des tests d'intégration.

Scénarios obligatoires :

1. Approche d'une zone dans le bon sens de circulation.
2. Circulation dans le sens opposé.
3. Route parallèle proche.
4. Absence d'itinéraire actif.
5. Recalcul d'itinéraire.
6. Mauvaise précision GPS.
7. Passage répété dans la même zone.
8. Sortie puis nouvelle entrée.
9. Passage hors connexion Internet.
10. Importation d'une base invalide ou corrompue.
11. Redémarrage de l'application avec conservation des paramètres.
12. Utilisation en arrière-plan durant une navigation active.

Prévoir si possible des tests utilisant des traces GPS simulées, sans nécessiter de déplacements réels.

## Phase 8 — Compilation et livraison

- Suivre les instructions officielles de compilation Android du dépôt.
- Vérifier les versions du SDK, NDK, CMake et du JDK définies par le projet.
- Préserver le fonctionnement des variantes de compilation existantes.
- Générer, si l'environnement le permet, un APK Debug ARM64.
- Adapter l'`applicationId` et le nom de l'application pour permettre l'installation du fork à côté d'Organic Maps officiel, sans compromettre les mécanismes de stockage ou de mise à jour.
- Documenter les limitations et les éventuelles différences avec la version officielle.
- Respecter les licences du projet et des bases géographiques utilisées.

Livrables attendus :

- Code source modifié.
- Tests automatisés.
- `docs/ZONE_DANGER_ANALYSIS.md`
- `docs/ZONE_DANGER_IMPLEMENTATION.md`
- Instructions de compilation Windows 11.
- APK Debug si la compilation est réalisable.

## Consignes de réalisation

Procède directement à l'analyse puis à l'implémentation.

Travaille par étapes cohérentes, avec des modifications ciblées et maintenables.

Ne réécris pas des composants qui existent déjà.

Ne modifie pas inutilement le calcul d'itinéraire, le moteur de rendu ou les fichiers de cartographie.

N'invente aucune API, aucun chemin de fichier et aucune capacité d'Organic Maps : vérifie leur existence dans le dépôt.

Si une exigence est incompatible avec l'architecture existante ou les contraintes réglementaires, documente le problème et implémente l'alternative techniquement viable la plus simple.

Ne te limite pas à produire un plan : réalise les modifications, exécute les tests disponibles et indique précisément ce qui a été validé et ce qui ne l'a pas été.

## Deux points techniques importants

- Le dépôt officiel documente la compilation Android sous Windows, l'utilisation des sous-modules Git, ainsi que des variantes `googleDebug` et `fdroidDebug`.&#x20;

  [image](https://www.google.com/s2/favicons?domain=https://github.com\&sz=32)

  GitHub

- Organic Maps dispose bien d'une commande de débogage `?debug-cam` qui force l'activation des radars, mais cela ne démontre pas que le système existant est directement utilisable pour des zones de danger conformes à la réglementation française.&#x20;

  [image](https://www.google.com/s2/favicons?domain=https://git.mitgai.net\&sz=32)

  GitHub



Je recommande de conserver la gestion des alertes dans le moteur de navigation existant, plutôt que de créer un service GPS Android indépendant. Cela évitera de gérer deux fois la localisation, réduira la consommation de batterie et facilitera les futures mises à jour du fork.

Source du projet : [Organic Maps — GitHub](https://github.com/organicmaps/organicmaps?utm_source=chatgpt.com).