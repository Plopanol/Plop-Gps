
# Quelle source de Radar GPS France puis trouvé gratuitement et legalement 

Oui. Il existe une base officielle, gratuite et légalement réutilisable pour ton fork d'Organic Maps : celle du ministère de l'Intérieur sur data.gouv.fr.

C'est celle que je te recommande en priorité.

## 1. Les meilleures sources

[Nouveaux pics de consultation pour la plateforme de données publiques Data.gouv.fr en 2020                        - Labo](https://images.openai.com/static-rsc-4/w-LtiIjfZdF6dXWpqh3VgjJ5TkNIhDGO2EOPmzuSpHIP_uCGsBHeRDFUn5wO9XB3dmIOtmMYCym0JA5f1ncfc1_KChQbduyjWj5eXi7FLtHAigBER4lEonsXDiPKFXCRCQNaOryrKK5t6S3L_JH4Vwcxv0KuSFRAS8wUON-9MJ8?purpose=inline)

1\. data.gouv.fr — Base officielle

Recommandée

Ministère de l'Intérieur · Mise à jour du 30 décembre 2025

Environ 3 400 radars fixes français, avec coordonnées GPS, type, vitesse maximale autorisée et date de mise en service.

Licence : Licence Ouverte 2.0, autorisant la réutilisation, y compris dans une application, avec attribution.

[Télécharger le CSV officiel](https://www.data.gouv.fr/datasets/liste-des-radars-fixes-en-france?utm_source=chatgpt.com)

[image](https://www.google.com/s2/favicons?domain=https://www.data.gouv.fr\&sz=32)

data.gouv.fr

[File:OpenStreetMap U.S. logo.svg - Wikimedia Commons](https://images.openai.com/static-rsc-4/6ZyerZN9mPIk0awk_Dj6yzt2E9zrEaztf9o3CQi7qBVA-jyQwa0TSPXMt-9sZxPbCrvvz2FA0Mlt4QDV0pSqBRSCxQyO0s-GYjFwRd9XkCXhlud7x1aww5uf6pu74shZPS-cjFrYFuuD7kUVWl0e3sYt6oSwUdYeT2qEO2M4B78?purpose=inline)

2\. OpenStreetMap — Base communautaire

Informations sur les radars au travers des tags `highway=speed_camera` et des relations `enforcement`.

Licence : ODbL 1.0, gratuite, avec obligations d'attribution et de partage à l'identique pour certaines bases dérivées.

[Extraire les données avec Overpass Turbo](https://overpass-turbo.eu/?utm_source=chatgpt.com)

[image](https://www.google.com/s2/favicons?domain=https://www.openstreetmap.org\&sz=32)

OpenStreetMap+1

[Explore speed camera maps and downloads by country | SpeedCams.world](https://images.openai.com/static-rsc-4/bcGVJHNvFG1-fn-mqS8HHybb7SLnegmQpE9bEI9selfsh_5Xy6KRo10RpmTJXzsG1KDQsMKl6cst7LGuB-vrTWCIlaDPWjbrtksdGP60JDEYqKC1tTp31g6kliPkuZHNoisbwRX1SUfCzSiC76BIEvQ9amGGYbRLTDdRLGYa9NA?purpose=inline)

3\. SpeedCams.world — Exports prêts à utiliser

3 822 emplacements déclarés en France, disponibles aux formats CSV, GPX et KML.

Licence : ODbL 1.0.

Attention : l'éditeur indique que la date de mise à jour des données est inconnue. Cette source est donc moins fiable pour une application de navigation.

[Télécharger les données France](https://speedcams.world/download?utm_source=chatgpt.com)

[image](https://www.google.com/s2/favicons?domain=https://speedcams.world\&sz=32)

SpeedCams.world

## 2. Comparaison pour ton fork

| Critère                     | data.gouv.fr | OpenStreetMap        | SpeedCams            |
| --------------------------- | ------------ | -------------------- | -------------------- |
| Gratuit                     | Oui          | Oui                  | Oui                  |
| Réutilisation autorisée     | Oui          | Oui, sous conditions | Oui, sous conditions |
| Export CSV                  | Oui          | Avec conversion      | Oui                  |
| Coordonnées GPS             | Oui          | Oui                  | Oui                  |
| Source officielle française | Oui          | Non                  | Non                  |
| Fraîcheur identifiable      | Déc. 2025    | Selon extraction     | Inconnue             |
| Intégration Organic Maps    | Facile       | Très pertinente      | Facile               |

## 3. Architecture conseillée

Pour ton développement, je te suggère de commencer uniquement avec la base du ministère de l'Intérieur.

CSV officiel — data.gouv.fr

Radars fixes, coordonnées, types, vitesses

Convertisseur local

Validation, association au réseau routier, regroupement en zones

Base de zones de danger

GeoJSON ou SQLite, sans positions précises exposées

Organic Maps — Android

Alertes vocales, direction de circulation, fonctionnement hors ligne

## 4. Point juridique important

La publication officielle des coordonnées des radars n'autorise pas automatiquement leur utilisation comme avertisseur de position exacte en conduite.

L'article R413-15 du Code de la route interdit notamment les dispositifs informant de la localisation des appareils de contrôle.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.legifrance.gouv.fr\&sz=32)

Légifrance+1



Pour ton fork, il faut donc distinguer la source de données, dont la réutilisation est autorisée, et le comportement des alertes, qui doit être conforme à la réglementation française. Transformer des points en zones ne garantit pas, à lui seul, cette conformité.

Ma recommandation : utilise `data.gouv.fr` comme référentiel principal, puis OpenStreetMap pour associer les données aux routes, aux directions et aux limitations de vitesse. Cela te permettra de construire un système entièrement hors ligne, gratuit et maintenable, sans dépendre d'une base commerciale.