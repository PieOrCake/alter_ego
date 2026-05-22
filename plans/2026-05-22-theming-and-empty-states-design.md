# Theming Consistency + H&S Empty State — Design

Date: 2026-05-22
Status: approved, ready for implementation plan

## Goals

1. **Theming consistency audit** — bring stragglers (account selector combo, plain action buttons, Options tab) into the gold-on-dark vocabulary used elsewhere, and walk every tab to flag drift in section headers, chip strips, and progress bars.
2. **First-run empty state** — replace terse grey "Hoard & Seek is required" text with a centered themed card containing icon, headline, explanation, and primary/secondary action buttons.

Out of scope (deferred):
- Loading state polish (skeletons / spinners)
- Per-tab empty data states (no builds, no clears, no achievements)
- Failed fetch / error state cards
- Portrait visual weight on Equipment tab (user controls dimming)

## Part A — Theming Consistency Audit

### Helper layer

New reusable UI helpers, added to `src/dllmain.cpp` (move to `src/UIHelpers.cpp` only if the file growth justifies it):

| Helper | Purpose |
|---|---|
| `RenderGoldButton(label, size)` | Gold-trim filled button for primary actions (Import, Save to Library, Copy Build Chat Link). Matches the gold-bar header vocabulary. |
| `RenderChipButton(label, active, accentColor)` | Consolidates the de facto chip pattern (mode chips, filter chips). One source of truth so retry buttons, account selector chips, etc. can reuse. |
| `RenderThemedCombo(label, current, items, count)` | Gold-bordered combo to replace plain `ImGui::Combo` (used for account selector). |
| `RenderEmptyCard(icon, headline, body, primaryLabel, primaryCB, secondaryLabel, secondaryCB)` | Centered themed empty-state card (see Part B). |

Helpers are file-static, no public API change.

### Audit checklist

Walk every tab; replace stragglers and note drift:

1. **Top window chrome** — account selector combo → `RenderThemedCombo`.
2. **Build editor** (`build.png`) — `Copy Build Chat Link`, `Save to Library` → `RenderGoldButton`.
3. **Build Library** — `+ Import Build` / `Cancel Import` / `Import` / `Done` (edit) → gold/chip buttons. Chat-addon status text ("Chat: Connected" / "Chat: Not detected") → small inline status chip with coloured dot.
4. **Achievements** — spot-check section headers, chip strip (Hide Complete), progress bars.
5. **Skinventory** — spot-check rarity banner card, filter chips, view toggle.
6. **Vault & Clears** — spot-check banner rows, fractal tier rows, AA pills.
7. **Options tab** — full pass: section headers using `RenderSectionHeader`, links re-styled to match.
8. **Relay popups** (`Build Sent`, `Send Failed`, `Sending to Mobile`) — `Copy Code` / `Done` / `OK` buttons → themed (gold for primary, plain chip for secondary).

Anything else found drifting during the audit is logged in the commit message and either fixed inline or noted for follow-up.

## Part B — H&S Not Detected Empty State

### Trigger condition

Existing H&S detection check (inter-addon event handshake). The card is shown when H&S is not detected, in any tab whose content requires H&S data.

Tabs that show the card when H&S is missing:
- Characters / Equipment
- Build Library (gated portion — Import still usable since it doesn't need H&S)
- Skinventory
- Vault & Clears
- Achievements

Tabs that don't gate on H&S stay as-is.

### Card structure

Centered in the available content region of each tab:

- 64px Alter Ego / H&S-themed icon (top)
- Bold headline: **"Hoard & Seek is required"**
- 2-line body: "Alter Ego reads your characters, equipment, and inventory through the Hoard & Seek companion addon."
- Two buttons side-by-side:
  - Primary (gold): **Retry Detection** — re-runs the H&S handshake
  - Secondary (chip): **Open Nexus Library** — opens the in-game Nexus addon library if the Nexus API exposes that; otherwise falls back to a copy-link button with the Nexus library URL
- Card chrome: gold border, dark fill, subtle drop shadow — matches the rarity banner card already used in Skinventory preview

### Behaviour

- Card auto-dismisses when H&S is detected (no re-render needed; gate re-evaluates per frame).
- Retry Detection button calls the existing H&S detection routine; success is signalled to the user via the existing Nexus alert mechanism (`GUI_SendAlert`).
- If the Nexus library "open library" function isn't available, the secondary button is replaced with a "Copy Library URL" chip-style action.

## File touch list (estimated)

- `src/dllmain.cpp` — helper functions added; audit replacements throughout; H&S empty card integration into each tab's render entry.
- Possibly `src/UIHelpers.cpp` + header if helper count or LoC grows past ~150 lines.

No data-layer changes. No `GW2API` / `HoardAndSeek` API changes.

## Validation

Manual, in-game (per project — no test suite):

1. Build cleanly compiles after each chunk.
2. With H&S installed: every tab looks identical to before except for the re-themed buttons/combos.
3. With H&S not installed: every gated tab shows the centered card; Retry Detection works once H&S is loaded.
4. Visual side-by-side check against screenshots in `screenshots/`.
