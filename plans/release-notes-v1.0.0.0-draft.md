# Alter Ego v1.0.0.0 — Release Notes (Draft)

## The 1.0 release.

A focused polish pass: nothing in this release is brand new functionality. The goal was to make every tab look and feel like one consistent product before tagging 1.0.

## What's New

- **Trait grid redesign.** Each trait line now shows the spec's banner art behind the row, with the spec name overlaid in gold. The standalone hex spec icon has been retired in favor of the banner as the visual anchor.
- **Theming consistency.** Every action button, dropdown, and section header across the addon now uses the same gold-on-dark vocabulary. The account selector, build editor buttons, build library import controls, relay popups, and the Options tab have all been brought into line.
- **First-run empty card.** When Hoard & Seek isn't detected, every tab now shows an inviting themed card with "Retry Detection" and "Copy Library URL" actions, instead of a terse warning line.
- **Animated loading spinners.** Plain "Loading..." text in profession data, weapon data, wiki images, hero-challenge data, the search index, and the character list has been replaced with a small rotating spinner + label.
- **Character header.** Age, birthday, last login, and crafting professions are now icon-driven (Tome of Knowledge / Birthday Gift / Chest of Loyalty / discipline icons) instead of long text labels — fits all 9 crafting disciplines without overflow.
- **Options tab pass.** Section headers now use the gold-bar vocabulary; "Compact mode" → "Expanded mode" with all column toggles always visible (greyed when expanded mode is off); cleaner first-run defaults for new users.

## Tweaks

- Higher-resolution wiki icons for trait-line hex graphics (256×256 instead of 64×64).
- Spec background banners are zoomed and bottom-anchored to bring subjects into the visible row. 24 specs whose source art is mostly transparent at the top get an extra zoom factor to fill the row properly.
- Marching-ants dotted line from the spec hex to the first trait removed (the hex is gone).
- Relay popup buttons (Copy Code / Done / OK) themed.
- "Show Quick Access icon" and other settings get sensible defaults for new installations.

## Internal

- New helpers: `RenderGoldButton`, `RenderChipButton`, `RenderThemedCombo`, `RenderEmptyCard`, `RenderSpinner` — one source of truth for the addon's UI vocabulary.

## Install

Drop `AlterEgo.dll` into `<GW2>/addons/AlterEgo.dll`.

Requires [Hoard & Seek](https://github.com/PieOrCake/hoard_and_seek) and [Raidcore Nexus](https://raidcore.gg/Nexus).
