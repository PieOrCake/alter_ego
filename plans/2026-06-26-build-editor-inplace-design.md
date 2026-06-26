# Build Editor — In-Place Editing (Design)

Date: 2026-06-26
Branch: `feature/build-editor`
Status: design, pre-implementation

## Goal

Finish the build editor by replacing the separate, cramped editor block with **in-place
editing of the existing build card**. When the user enters edit mode, the same rich
`RenderSavedBuildPreview` layout (spec banners, minor traits, animated "marching ants"
connectors, skill row) simply **unlocks**: traits, skills, gear, spec choices and the
header fields all become editable in situ. Nothing floats above the card.

This collapses two stacked layouts (cramped editor + rich preview) into one.

## Decisions (settled in brainstorming)

1. **One in-place card.** Delete the cramped editor block and the duplicate trait grid;
   `RenderSavedBuildPreview` renders both view and edit modes.
2. **Marching ants reroute to the current pick.** No memory of the saved selection — the
   connector simply follows whatever trait is selected now. (Editing a trait re-routes the
   line; that *is* the visual feedback.)
3. **Header chrome lives in the header**, in place (name field, mode chips, notes,
   Save/Cancel) — no strip above the card.
4. **Skill picker restyled** to match Pie UI's native-style flyout grid.
5. **Dirty flag + buffer-everything.** Cancel genuinely reverts to the pre-edit state.

## Architecture

### 1. `RenderSavedBuildPreview` gains an `editable` path

Signature gains a flag, e.g. `RenderSavedBuildPreview(const SavedBuild& build, bool
showEditButton, bool editable)`. When `editable`:

- It renders against `g_EditDraft` (the mutable working copy) rather than the const build.
- Trait/skill/gear/spec icons become click targets (see below).
- The header renders the editable chrome instead of the static title row.

When not `editable`, behaviour is byte-identical to today (the view card is untouched).

Call site ([~line 9134](../src/dllmain.cpp)) passes `editable = g_LibEditMode`.

### 2. Trait grid unlocks in place

In the existing 6-column inner table, each **major** trait icon gets an invisible button
over it when `editable`. Click sets `g_EditDraft.specializations[i].traits[tier]` to that
trait id. The selected/dimmed visuals and the connector routing are unchanged — the
connector re-routes automatically because it already tracks the selected centre.

Minor traits stay fixed (always-on, not clickable).

`RenderTraitIcon` gains an optional `editable` parameter that overlays an `InvisibleButton`
and reports clicks, while still returning the icon centre for connector math. The standalone
`RenderEditableTraitIcon` helper and the cramped 3×3 grid are deleted.

### 3. Spec selection in the rich layout

- The **spec name / banner** region becomes clickable when `editable` → sets
  `g_SpecPickerSlot = i`, opening the existing `RenderSpecPickerDialog`.
- An **empty spec slot** (`spec_id == 0`) renders a "Choose specialization" affordance
  (clickable) instead of the static "(empty)" text.

### 4. Header chrome, in place

In `editable` mode the card's header row swaps:

- Title "Core Bonkadin" → inline `InputTextString` field.
- "· PvE" → the existing clickable game-mode chip strip, but pointed at the **draft**
  (`g_EditDraft.game_mode`) instead of persisting immediately.
- Share / Edit / Delete buttons → **Save / Cancel**.
- Notes → an `InputTextMultilineString` just under the header.

### 5. Skill picker restyle (Pie UI native-style grid)

Reference: `../pie_ui/src/widgets/SkillbarWidget.cpp` (right-click skill-alternates grid)
and `../pie_ui/src/skills/SkillCandidates.h`.

`RenderSkillPickerDialog` keeps all current filter/apply logic (type, elite-gate,
no-duplicate-utility, aquatic mirror) but the list body changes from a vertical
icon+name `Selectable` list to a **grid**:

- 4 columns, ~40px cells.
- Each **skill category** (Signet, Shout, Meditation…) starts a fresh row, mirroring the
  in-game flyout. *Depends on `SkillInfo` carrying a category field — see Open Items. If it
  doesn't, the grid still works without per-category row breaks.*
- The currently-equipped skill gets a thick border; hover shows the full tooltip.
- Left-click a skill applies it; "(None)" clears the slot.

The Revenant **legend** and Ranger **pet** pickers get the same grid treatment for
consistency (they already have working pickers with the right data).

### 6. Gear unlocks via the existing dialog

Equipment/weapon slots in the card become click targets that open the existing
`RenderGearCustomizeDialog`. *Exact wiring depends on how gear currently renders inside the
card — see Open Items.*

### 7. Dirty flag + Save/Cancel (true revert)

Today, name/notes (`UpdateSavedBuild`) and game mode (`SetSavedBuildGameMode`) persist
**immediately**, so there is no clean pre-edit state. Fix:

- **Enter edit mode:** the on-disk build is the snapshot. Seed `g_EditDraft`, the name/notes
  buffers and the draft game-mode from it. Reset `g_EditDirty = false`.
- **While editing:** *every* change writes only to the draft/buffers — never to disk. Remove
  the live `UpdateSavedBuild` calls on the name/notes fields; point the mode chips at
  `g_EditDraft.game_mode`. Any edit sets `g_EditDirty = true`.
- **Save:** persist all at once — `UpdateSavedBuild(name, notes)` +
  `SetSavedBuildGameMode(draft mode)` + `ReplaceSavedBuildDefinition(g_EditDraft)`. Clear
  dirty, force re-sync, leave edit mode. Save is enabled only when dirty.
- **Cancel:** if `g_EditDirty`, pop a small "Discard changes?" confirm (consistent with the
  existing Delete confirmation). On confirm (or if not dirty) discard the draft and leave
  edit mode — the on-disk build was never touched, so the card is exactly as before.

### 8. Edit-mode visual differentiation

Because edit mode reuses the view card, the mode must be **obvious**. Three layered cues:

- **Animated card border (anchor).** A gold/amber **marching-ants** frame wraps the entire
  build card while editing, reusing the existing `DrawDottedLine` animation from the trait
  connectors (same motif). Impossible to miss; frames exactly the editable surface. Gold
  matches the Save button so the frame and primary action read as one "edit" colour.
- **"● EDITING" header badge.** An amber pill in the header next to the (now editable) name
  field. Makes the *state* explicit, alongside Save/Cancel replacing Share/Edit/Delete.
- **Per-element hover highlights.** Every editable element (traits, skills, gear, spec
  banner) brightens and draws a thin accent outline on hover, signalling "click me" — the
  static view never does this, so it tells the user *where* to act.

Optional polish (not required): a subtle warm tint on the card background.

Red is deliberately avoided (reserved for Delete/danger).

## What gets deleted

- The cramped `===== Edit mode =====` Specializations/Skills/Legends/Pets block
  ([~lines 9023–9114](../src/dllmain.cpp)).
- `RenderEditableTraitIcon` and its duplicate 3×3 grid.
- The "Done" button and the live-persist name/notes/mode writes.

## Open items to verify during implementation

1. **`SkillInfo` category field** — does it carry a skill-category string (Signet/Shout/…)
   for the per-category row breaks? If not, group is skipped (grid still fine).
2. **Gear rendering in the card** — confirm how weapon/armor slots are drawn inside
   `RenderSavedBuildPreview` so the click-to-open-gear-dialog wiring lands cleanly.
3. **`ReplaceSavedBuildDefinition` vs game mode** — confirm game mode is applied via
   `SetSavedBuildGameMode` on Save and not lost by the definition replace.

## Invariants

- **View mode is byte-identical** to today (the `editable = false` path is unchanged).
- **No disk write happens until Save.** Cancel leaves the on-disk build untouched.
- **Build clean** after every task (`cd build && make -j$(nproc)`), per project rule.
- No new threading around `Events_Raise` / `s_mutex` (all edits are render-thread, in-memory).

## Task order (each ends with a clean build)

1. `RenderSavedBuildPreview` editable flag + render against draft; trait grid clickable;
   delete cramped block + `RenderEditableTraitIcon`.
2. Header chrome in place (name/mode/notes/Save/Cancel) + dirty flag + buffer-everything +
   remove live-persist writes + Cancel confirm.
3. Spec-banner click → spec picker; empty-slot affordance.
4. Skill picker grid restyle (+ legends/pets); verify category field.
5. Gear slot click → gear dialog.
6. Edit-mode visual cues: animated card border + "EDITING" badge + per-element hover
   highlights.
7. In-game validation against the invariants.
