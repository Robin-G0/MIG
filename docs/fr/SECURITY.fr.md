# Sécurité

[English](../../SECURITY.md) | [Français](SECURITY.fr.md)

## Profils importés

MIG associe les mouvements à des séquences clavier ; il n'exécute pas directement
des commandes shell. Raccourcis et texte peuvent agir sur le système ou l'application
active. L'import desktop affiche les correspondances et désactive la sortie clavier.
Validez le profil après lecture, puis activez séparément cette sortie.
Le label **System interaction** est une heuristique, pas une garantie de sécurité.
Par input : maximum 256 actions sérialisées, 1 024 touches de raccourci et
16 384 octets UTF-8 de texte. Les valeurs invalides sont rejetées sans troncature.
[Revue d'import](../reference/configuration.fr.md#revue-dimport).

Signaler les vulnérabilités en privé. Si **Report a vulnerability** apparaît dans
l'onglet Security du dépôt, utiliser GitHub Private Vulnerability Reporting.
Sinon, ouvrir une issue demandant un canal privé sans détails d'exploitation,
identifiants ni configuration privée.

Préciser version/plateforme, reproduction, impact et exemple minimal anonymisé.
Pendant l'analyse, éviter d'exécuter un profil téléchargé avec la sortie clavier
active : les profils peuvent configurer des séquences de touches volontaires.

Les corrections ciblent la branche 1.x actuelle. Les intégrations Preview et
les dépendances caméra/modèles ont leurs propres limites de validation ; voir la
[matrice de support](../reference/support.fr.md). Aucun délai de réponse n'est garanti.

Les administrateurs peuvent activer signalement privé, secret scanning, push
protection et contrôles CI requis dans GitHub. Cette politique ne les active pas.
Voir le [signalement privé GitHub](https://docs.github.com/en/code-security/security-advisories/working-with-repository-security-advisories/configuring-private-vulnerability-reporting-for-a-repository).
