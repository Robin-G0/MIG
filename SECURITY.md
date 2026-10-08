# Security

[English](SECURITY.md) | [Français](docs/fr/SECURITY.fr.md)

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

## Imported movement profiles

MIG maps movements to keyboard inputs, including simultaneous chords and ordered
typed text. It does not directly execute shell commands. Keyboard shortcuts and text
can indirectly interact with the operating system or focused application.
Desktop file imports show every movement/action sequence for review before replacing
the profile, and turn keyboard output off. Review the sequence, import it explicitly,
then enable Keyboard output separately. Cancel retains the current profile.

Windows/Super, window/session shortcuts and launch keys receive a **System interaction**
label. This is a warning heuristic, not a sandbox or a guarantee of safety.
SDK and browser examples emit logical actions; their applications decide what to do.
Every parser/runtime validates action definitions and rejects unsupported/reserved
keyboard codes and mouse/gamepad codes in keyboard actions. One input is limited to
256 serialized actions, 1,024 chord-key entries and 16,384 UTF-8 text bytes. Actions
are rejected rather than truncated. See [configuration review](docs/reference/configuration.md#import-review).
