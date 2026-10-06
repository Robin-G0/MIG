# Security

[English](SECURITY.md) | [Français](SECURITY.fr.md)

Report vulnerabilities privately. If **Report a vulnerability** is available in
this repository's Security tab, use GitHub Private Vulnerability Reporting.
If it is unavailable, open an issue requesting a private contact channel without
including exploit details, credentials or private configuration files.

Include the affected version/platform, reproducible steps, observed impact and a
small sanitized example. Avoid executing downloaded profiles with keyboard output
enabled while investigating; profiles can configure deliberate keyboard sequences.

Security fixes target the current 1.x line. Preview engine integrations and
upstream camera/model dependencies have their own validation limits; see the
[support matrix](docs/reference/support.md). No response-time guarantee is stated.

Repository administrators can enable private reporting, secret scanning, push
protection and required CI checks in GitHub settings. Those settings are not
enabled by committing this policy. See [GitHub private reporting](https://docs.github.com/en/code-security/security-advisories/working-with-repository-security-advisories/configuring-private-vulnerability-reporting-for-a-repository).
