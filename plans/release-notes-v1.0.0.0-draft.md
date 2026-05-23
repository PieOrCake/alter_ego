# Alter Ego v1.0.0.0

The 1.0 release.

## What's New

- **UI polish.** Trait grids now show the spec banner art behind the row. Theming, buttons, dropdowns, spinners, and empty states are consistent across every tab. Character headers are icon-driven. Options tab tidied. Account selector moved onto the tab row to save vertical space.

## Fixes

- Vault summary totals now include the daily 20 AA and weekly 450 AA meta-completion bonuses.
- Achievement progress is cached per-account and reloaded on startup (instant rail counts).
- Pinned achievements refresh every 10 minutes, regardless of which tab is open.

## Security

- TLS hostname checks enforced on all HTTPS requests.
- Character portrait paths sanitized before disk access.
- Relay codes re-validated before being used in the AE2 relay URL.

## Install

Drop `AlterEgo.dll` into `<GW2>/addons/AlterEgo.dll`.

Requires [Hoard & Seek](https://github.com/PieOrCake/hoard_and_seek) and [Raidcore Nexus](https://raidcore.gg/Nexus).
