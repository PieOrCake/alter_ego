# Build Editor — In-Place Editing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the cramped separate build-editor block with in-place editing of the rich build card, add a true dirty-flag Save/Cancel model, restyle the skill picker to a grid, and make edit mode visually obvious.

**Architecture:** `RenderSavedBuildPreview` (the rich card in [src/dllmain.cpp](../src/dllmain.cpp)) gains an editable path that renders against the mutable working copy `g_EditDraft`, makes traits/skills/spec/gear clickable, and draws edit-mode chrome (name field, mode chips, notes, Save/Cancel) in the header. All edits buffer into `g_EditDraft` + name/notes buffers; nothing persists until Save. The old cramped editor block and `RenderEditableTraitIcon` are deleted.

**Tech Stack:** C++17, Dear ImGui 1.80, Nexus addon API, MinGW cross-compile. No unit-test suite — each task verifies by **clean build** (`cd build && make -j$(nproc)`) plus a described in-game/visual check.

## Global Constraints

- Build after every task: `cd build && make -j$(nproc)` — must compile clean (no new warnings).
- Never deploy/copy the DLL into the game; the user deploys.
- View mode (`g_LibEditMode == false`) must render **byte-identical** to today.
- No disk write may happen until Save is pressed; Cancel must leave the on-disk build untouched.
- All editing runs on the render thread, in-memory — no new `Events_Raise`/`s_mutex` interaction.
- Reuse existing UI helpers (`RenderChipButton`, `RenderGoldButton`, `RenderSectionHeader`, `InputTextString`, `InputTextMultilineString`, `DrawDottedLine`).
- Spec: [plans/2026-06-26-build-editor-inplace-design.md](2026-06-26-build-editor-inplace-design.md).

## File Structure

All work is in [src/dllmain.cpp](../src/dllmain.cpp) (the UI layer). No new files. Key regions:
- Edit-state globals: ~line 731 (`g_EditDraft`, `g_EditDraftId`, picker slots, `g_LibEditMode`, `g_LibEditName/Notes`).
- `RenderTraitIcon` ~3818; `RenderEditableTraitIcon` ~3872 (to delete).
- Pickers: `RenderSpecPickerDialog` ~6550, `RenderSkillPickerDialog` ~6607, legend ~6677, pet ~6713, gear ~6754.
- `RenderSavedBuildPreview` ~7766 (header ~7766-7912, trait grid ~7936-8130, skills/gear below).
- Edit-mode entry + cramped block: ~8950-9134 inside `RenderBuildLibrary`.

---

### Task 1: Edit-state plumbing — dirty flag, snapshot, enter/leave helpers

**Files:**
- Modify: `src/dllmain.cpp` (globals ~731; edit-entry ~8950-8970; Edit button ~7877).

**Interfaces:**
- Produces: `bool g_EditDirty`; `void EnterBuildEditMode(const AlterEgo::SavedBuild& build)`; `void MarkEditDirty()`. Later tasks call `MarkEditDirty()` on every edit and gate Save on `g_EditDirty`.

- [ ] **Step 1: Add globals** near the existing edit globals (~line 733):

```cpp
static bool g_EditDirty = false;            // true once the draft diverges from the snapshot
```

- [ ] **Step 2: Add helpers** just above `RenderSavedBuildPreview` (~line 7760):

```cpp
// Seed the editing draft + name/notes buffers from a build and clear the dirty flag.
// The on-disk build is itself the pre-edit snapshot (nothing persists until Save), so
// Cancel just discards the draft.
static void EnterBuildEditMode(const AlterEgo::SavedBuild& build) {
    g_EditDraft = build;
    g_EditDraftId = build.id;
    g_LibEditName = build.name;
    g_LibEditNotes = build.notes;
    g_EditDirty = false;
    g_LibEditMode = true;
}
static inline void MarkEditDirty() { g_EditDirty = true; }
```

- [ ] **Step 3: Route the Edit button through the helper.** At ~line 7877 replace:

```cpp
if (ImGui::SmallButton("Edit##headerBuild")) g_LibEditMode = true;
```
with:
```cpp
if (ImGui::SmallButton("Edit##headerBuild")) EnterBuildEditMode(build);
```

- [ ] **Step 4: Keep the existing draft-sync from clobbering the draft mid-edit.** At ~line 8966 change:

```cpp
if (g_EditDraftId != build.id) { g_EditDraft = build; g_EditDraftId = build.id; }
```
to only re-seed when NOT actively editing:
```cpp
if (!g_LibEditMode && g_EditDraftId != build.id) {
    g_EditDraft = build; g_EditDraftId = build.id;
}
```

- [ ] **Step 5: Build.** Run: `cd build && make -j$(nproc)` — expect clean compile. In-game unchanged (helper not yet driving behaviour beyond Edit entry).

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: dirty flag + edit-mode enter/snapshot helpers"
```

---

### Task 2: Editable trait grid in the rich card + delete cramped block

**Files:**
- Modify: `src/dllmain.cpp` — `RenderTraitIcon` ~3818; trait grid in `RenderSavedBuildPreview` ~7929-8130; delete `RenderEditableTraitIcon` ~3872-3905; delete cramped Specializations/Skills/Legends/Pets block ~9023-9114.

**Interfaces:**
- Consumes: `g_EditDraft`, `g_LibEditMode`, `MarkEditDirty()`.
- Produces: trait grid edits write `g_EditDraft.specializations[i].traits[tier]`.

- [ ] **Step 1: Give `RenderTraitIcon` an editable click path.** Change its signature (~3818) to add a trailing flag and return click via an out-param is overkill — instead overlay an InvisibleButton when editable and stash "clicked" in a file-static. Simpler: add params `bool editable=false, bool* clicked=nullptr`. At the top, when `editable`, draw an `InvisibleButton` over the icon rect FIRST (so it owns the click), then draw the image via the window draw list. Replace the `ImGui::Image(...)` calls with `ImGui::GetWindowDrawList()->AddImage(...)` and set `*clicked` from the button. Keep the non-editable path exactly as-is (still `ImGui::Image`, no button) to preserve OOTB. Add a **hover highlight** when editable: on `ImGui::IsItemHovered()`, draw a thin gold outline + brighten.

```cpp
static ImVec2 RenderTraitIcon(uint32_t trait_id, bool selected, bool isMinor, float size,
                              bool editable = false, bool* clicked = nullptr) {
    const auto* tinfo = AlterEgo::GW2API::GetTraitInfo(trait_id);
    ImGui::PushID((int)(trait_id + 2000000));
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    bool hovered = false;
    if (editable) {
        if (ImGui::InvisibleButton("##et", ImVec2(size, size)) && clicked) *clicked = true;
        hovered = ImGui::IsItemHovered();
        ImGui::SetCursorScreenPos(pos); // reset so existing layout math below is unaffected
    }
    Texture_t* tex = AlterEgo::IconManager::GetIcon(trait_id + 2000000);
    if (tex && tex->Resource) {
        if (selected)
            dl->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(100,220,255,255), 0.0f,0,1.5f);
        int alpha = (!selected && !isMinor) ? 76 : 255;        // 0.3*255 dim for unselected majors
        if (editable && hovered) alpha = 255;                  // brighten on hover
        if (editable) {
            dl->AddImage(tex->Resource, pos, ImVec2(pos.x+size, pos.y+size),
                         ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,alpha));
        } else if (alpha != 255) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f); ImGui::Image(tex->Resource, ImVec2(size,size)); ImGui::PopStyleVar();
        } else {
            ImGui::Image(tex->Resource, ImVec2(size,size));
        }
        if (editable && hovered)
            dl->AddRect(pos, ImVec2(pos.x+size, pos.y+size), IM_COL32(227,196,122,255), 0.0f,0,2.0f);
    } else {
        ImVec4 col = selected ? ImVec4(0.3f,0.5f,0.7f,0.8f) : ImVec4(0.15f,0.15f,0.15f,0.5f);
        dl->AddRectFilled(pos, ImVec2(pos.x+size, pos.y+size), ImGui::ColorConvertFloat4ToU32(col));
        if (!editable) ImGui::Dummy(ImVec2(size, size));
        if (tinfo && !tinfo->icon_url.empty()) AlterEgo::IconManager::RequestIcon(trait_id+2000000, tinfo->icon_url);
    }
    // (tooltip block unchanged)
    ...
    ImGui::PopID();
    return center;
}
```

NOTE: when `editable`, the InvisibleButton already advances the cursor by `size`, so do NOT also call `ImGui::Dummy`. The `SetCursorScreenPos(pos)` after the button keeps the table-cell layout identical to the non-editable path.

- [ ] **Step 2: Drive editing from the grid.** In `RenderSavedBuildPreview`, the major-trait render at ~8080 currently is:

```cpp
ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz);
```
Replace with an editable-aware call that writes the pick:
```cpp
bool tClicked = false;
ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz, g_LibEditMode, &tClicked);
if (tClicked) {
    g_EditDraft.specializations[i].traits[tier] = (int)tid;
    MarkEditDirty();
}
```
Here `i` is the spec loop index and `spec` must refer to the draft when editing. At the top of the spec loop (~7923) make the source switchable:
```cpp
const auto& spec = g_LibEditMode ? g_EditDraft.specializations[i] : build.specializations[i];
```
Apply the same `g_LibEditMode ? g_EditDraft : build` selection anywhere the trait grid reads `build.specializations` / `spec.traits` (the selected-state math at ~8073-8079).

- [ ] **Step 3: Delete the cramped editor's Specializations block** (~9035-9049, the `if (si && si->major_traits.size() >= 9)` 3×3 grid and its surrounding per-slot chip loop) — but KEEP the spec **chip buttons** for now (Task 3 moves spec-picking to the banner). Actually delete the entire `for (int s=0; s<3; s++)` spec block at ~9028-9049; spec selection moves to the banner in Task 3. Leave Skills/Legends/Pets cramped blocks until Task 4.

- [ ] **Step 4: Delete `RenderEditableTraitIcon`** (~3872-3905) — now unreferenced. If the compiler reports it still referenced, the reference is the cramped grid from Step 3; remove that too.

- [ ] **Step 5: Build.** `cd build && make -j$(nproc)` — clean. In-game: enter edit mode, click majors in the rich card → selection + connector reroute live; view mode unchanged.

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: editable trait grid in the rich card; remove cramped trait grid"
```

---

### Task 3: Spec selection via banner click + empty-slot affordance

**Files:**
- Modify: `src/dllmain.cpp` — spec-name overlay in `RenderSavedBuildPreview` ~8004-8038; empty-slot branch ~7924; remove leftover cramped spec chips ~9028.

**Interfaces:**
- Consumes: `g_SpecPickerSlot`, `g_LibEditMode`. Produces: sets `g_SpecPickerSlot = i` to open `RenderSpecPickerDialog`.

- [ ] **Step 1: Make the spec name/banner clickable when editing.** The spec-name overlay already creates an `InvisibleButton("##specname", hitSz)` at ~8020 for the tooltip. When `g_LibEditMode`, treat a click on it as "open spec picker":

```cpp
ImGui::InvisibleButton("##specname", hitSz);
if (g_LibEditMode && ImGui::IsItemClicked()) g_SpecPickerSlot = i;
if (g_LibEditMode && ImGui::IsItemHovered())
    dl->AddRect(nameAnchor, ImVec2(nameAnchor.x+hitSz.x, nameAnchor.y+hitSz.y), IM_COL32(227,196,122,200), 2.0f);
```
(`i` = spec loop index.)

- [ ] **Step 2: Empty-slot affordance.** At ~7924 the empty branch is:

```cpp
if (spec.spec_id == 0) { ImGui::TextColored(..., "  (empty)"); continue; }
```
When editing, render a clickable chip instead:
```cpp
if (spec.spec_id == 0) {
    if (g_LibEditMode) {
        ImGui::PushID(i);
        if (RenderChipButton("+ Choose specialization", false)) g_SpecPickerSlot = i;
        ImGui::PopID();
    } else {
        ImGui::TextColored(ImVec4(0.4f,0.4f,0.4f,1.0f), "  (empty)");
    }
    continue;
}
```

- [ ] **Step 3: Mark dirty on spec change.** `RenderSpecPickerDialog` already writes `g_EditDraft.specializations[...]`. Add `MarkEditDirty();` at each mutation point inside it (~6584, ~6594) so spec changes flip the dirty flag.

- [ ] **Step 4: Remove leftover cramped spec chips** (~9028-9034) if any remain from Task 2 Step 3.

- [ ] **Step 5: Build + in-game:** click a spec banner in edit mode → picker opens; pick → grid repopulates; empty slot shows the chip. View mode unchanged.

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: spec selection via banner click + empty-slot affordance"
```

---

### Task 4: Skills/legends/pets unlock in the card + skill picker grid restyle

**Files:**
- Modify: `src/dllmain.cpp` — skill row in `RenderSavedBuildPreview` (find where heal/utility/elite are drawn, below the trait grid); `RenderSkillPickerDialog` ~6607-6673; delete cramped Skills/Legends/Pets blocks ~9051-9114.

**Interfaces:**
- Consumes: `g_SkillPickerSlot`, `g_LegendPickerSlot`, `g_PetPickerSlot`, `g_LibEditMode`.

- [ ] **Step 1: Make the card's skill icons clickable.** In the card's skill row, when `g_LibEditMode`, overlay an invisible button per heal/utility/elite icon and set the matching picker slot on click; add the gold hover outline. (Locate the skill row by searching for `terrestrial_skills.heal` rendering inside `RenderSavedBuildPreview`.) Reuse the slot numbering 0=heal,1-3=util,4=elite, and for Rev set `g_LegendPickerSlot`, Ranger `g_PetPickerSlot`. Call `MarkEditDirty()` is done inside the pickers (Step 3).

- [ ] **Step 2: Restyle `RenderSkillPickerDialog` to a grid.** Keep the filter/apply/gate/dup logic (~6618-6649). Replace the vertical `BeginChild` list (~6651-6667) with a 4-column grid built from the filtered ids:

```cpp
std::vector<uint32_t> shown;
for (uint32_t id : skillIds) {
    const auto* sk = AlterEgo::GW2API::GetSkillInfo(id);
    if (!sk || sk->type != wantType) continue;
    if (!eliteGateOk(sk->specialization)) continue;
    if (isDupUtil(id)) continue;
    shown.push_back(id);
}
const int COLS = 4; const float cell = 40.0f, pad = 4.0f;
ImGui::BeginChild("##skillgrid", ImVec2(COLS*(cell+pad)+pad, 360), true);
uint32_t equipped = (slot==0)? g_EditDraft.terrestrial_skills.heal
                  : (slot==4)? g_EditDraft.terrestrial_skills.elite
                  : g_EditDraft.terrestrial_skills.utilities[slot-1];
for (size_t k = 0; k < shown.size(); ++k) {
    if (k % COLS) ImGui::SameLine(0, pad);
    ImGui::PushID((int)shown[k]);
    ImVec2 p = ImGui::GetCursorScreenPos();
    if (RenderEditableSkillIcon(shown[k], cell)) applySkill(shown[k]);
    if (shown[k] == equipped)                         // equipped highlight
        ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x+cell,p.y+cell), IM_COL32(255,255,255,255), 0.0f,0,3.0f);
    ImGui::PopID();
}
if (shown.empty()) RenderSpinner("Loading skills...");
ImGui::EndChild();
if (ImGui::Button("(None)")) applySkill(0);
```
NOTE: skill-category row grouping is omitted — `SkillInfo` carries no category field (only `type`). Optional later follow-up: parse `/v2/skills` `categories` and break rows per category like Pie UI.

- [ ] **Step 3: Mark dirty on skill/legend/pet change.** Add `MarkEditDirty();` inside `applySkill` (~6625), the legend picker mutations (~6693, ~6701), and the pet picker mutations (~6735, ~6743).

- [ ] **Step 4: Delete cramped Skills/Legends/Pets blocks** ~9051-9114 (now redundant — the card row is the editor).

- [ ] **Step 5: Build + in-game:** click skill icons in the card → grid picker; pick → updates live; equipped highlighted. Rev legends / Ranger pets pickers open from the card. View mode unchanged.

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: skills/legends/pets editable in-card; skill picker grid restyle"
```

---

### Task 5: Header chrome in place (name/mode/notes/Save/Cancel) + buffer name/notes/mode + Cancel confirm

**Files:**
- Modify: `src/dllmain.cpp` — header in `RenderSavedBuildPreview` ~7846-7912 (title + buttons); edit-entry block ~8970-9130 (remove cramped name/notes/mode + Save/Done, replace with Save/Cancel logic and a confirm modal).

**Interfaces:**
- Consumes: `g_EditDirty`, `g_LibEditName`, `g_LibEditNotes`, `g_EditDraft`, `EnterBuildEditMode`.

- [ ] **Step 1: Editable title.** In the header, when `g_LibEditMode`, replace the `ImGui::TextColored(profColor, "%s", build.name)` title (~7847) with an inline input bound to `g_LibEditName`:

```cpp
if (g_LibEditMode) {
    ImGui::SetCursorScreenPos(ImVec2(textX, textTop));
    ImGui::SetNextItemWidth(220.0f);
    if (InputTextString("##edit_name", g_LibEditName)) MarkEditDirty();
} else {
    ImGui::SetCursorScreenPos(ImVec2(textX, textTop));
    ImGui::TextColored(profColor, "%s", build.name.c_str());
}
```

- [ ] **Step 2: Mode chips in the subtitle.** When editing, after the subtitle, draw the existing 5 mode chips (PvE/WvW/PvP/Raid/Fractal) bound to `g_EditDraft.game_mode`; clicking sets `g_EditDraft.game_mode` + `MarkEditDirty()` (NOT `SetSavedBuildGameMode`). Reuse the chip styling from the old block (~8988-9011).

- [ ] **Step 3: Header buttons swap.** In the `showEditButton` button cluster (~7858-7878): when `g_LibEditMode`, render **Save** (gold, disabled unless `g_EditDirty`) and **Cancel** instead of Share/Edit/Delete.

```cpp
// inside header button area:
if (g_LibEditMode) {
    // Save (enabled only when dirty)
    if (!g_EditDirty) ImGui::BeginDisabled();
    if (ImGui::SmallButton("Save##hdr")) {
        AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName.c_str(), g_LibEditNotes.c_str());
        AlterEgo::GW2API::SetSavedBuildGameMode(build.id, g_EditDraft.game_mode);
        AlterEgo::GW2API::ReplaceSavedBuildDefinition(build.id, g_EditDraft);
        g_EditDraftId.clear();   // force re-sync from persisted build
        g_LibEditMode = false; g_EditDirty = false;
    }
    if (!g_EditDirty) ImGui::EndDisabled();
    ImGui::SameLine(0,4);
    if (ImGui::SmallButton("Cancel##hdr")) {
        if (g_EditDirty) ImGui::OpenPopup("Discard changes?##editcancel");
        else g_LibEditMode = false;
    }
} else { /* existing Share/Edit/Delete cluster */ }
```

- [ ] **Step 4: Notes field.** When editing, draw the Notes input under the header (reuse `InputTextMultilineString("##edit_notes", g_LibEditNotes, ImVec2(0,60))` + `MarkEditDirty()` on change). Place it where the read-only notes line renders today (~8973 area), gated on `g_LibEditMode`.

- [ ] **Step 5: Cancel-confirm modal.** Add near the other modals in `RenderBuildLibrary`:

```cpp
if (ImGui::BeginPopupModal("Discard changes?##editcancel", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(ImVec4(0.85f,0.8f,0.7f,1.0f), "Discard your unsaved changes?");
    ImGui::Spacing();
    if (ImGui::Button("Discard", ImVec2(110,0))) {
        g_LibEditMode = false; g_EditDirty = false; g_EditDraftId.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Keep editing", ImVec2(110,0))) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}
```

- [ ] **Step 6: Delete the remaining cramped block** (Name/Mode/Notes/Save/Done ~8970-9130) — everything between `// ===== Edit mode =====` and the closing `}` before the `Separator()` / `RenderSavedBuildPreview` call. The card now IS the editor, so the surrounding `if (!g_LibEditMode) {...} else {...}` collapses: always call `RenderSavedBuildPreview(build, true)` (it internally branches on `g_LibEditMode`). Read-only notes stay as-is in the non-edit path inside the card.

- [ ] **Step 7: Build + in-game:** enter edit mode → name is an input, mode chips live, Save greyed until a change, Cancel with changes prompts discard, Cancel without changes just closes. Edit name/trait, Cancel → reselect build → original restored. View mode unchanged.

- [ ] **Step 8: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: in-place header chrome + true Save/Cancel with dirty flag"
```

---

### Task 6: Gear unlock — redirect the gear dialog through the draft (Cancel-safe)

**Files:**
- Modify: `src/dllmain.cpp` — `RenderGearCustomizeDialog` ~6754 (its `editBuild` lookup ~6764-6772); the card's gear slots in `RenderSavedBuildPreview`; gear-dialog open trigger.

**Interfaces:**
- Consumes: `g_EditDraft`, `g_LibEditMode`, `g_GearDialogBuildId`.

> **DECISION (flagged to user):** today the gear dialog mutates the **on-disk** build directly, which would bypass Cancel. This task redirects it to edit `g_EditDraft` while in edit mode so gear honours the dirty/Cancel model. If the user prefers to defer this, the fallback is: leave gear persisting immediately and document that Cancel reverts everything *except* gear.

- [ ] **Step 1: Point the dialog at the draft when editing.** In `RenderGearCustomizeDialog`, replace the `editBuild` lookup (~6764-6772) so that when `g_LibEditMode && g_GearDialogBuildId == g_EditDraft.id`, `editBuild = &g_EditDraft;` else fall back to the existing GetSavedBuilds() search. Ensure any "save gear" write inside the dialog is **skipped** when editing the draft (the draft persists only on Save Build); flag dirty via `MarkEditDirty()` instead.

- [ ] **Step 2: Make the card's gear slots open the dialog in edit mode.** In the card, when `g_LibEditMode`, the gear-slot icons become click targets that set `g_GearDialogBuildId = build.id; g_GearDialogOpen = true;` (matching how the slot dialog is opened today). Add the gold hover outline.

- [ ] **Step 3: Build + in-game:** edit gear in edit mode → changes show in the card; Cancel reverts gear too; Save persists gear. View mode unchanged.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: gear editing routed through the draft (Cancel-safe)"
```

---

### Task 7: Edit-mode visual cues — animated card border + "EDITING" badge

**Files:**
- Modify: `src/dllmain.cpp` — top and bottom of `RenderSavedBuildPreview` (border); header (badge).

**Interfaces:**
- Consumes: `g_LibEditMode`, `DrawDottedLine`.

- [ ] **Step 1: Capture the card's top-left at entry.** Near the start of `RenderSavedBuildPreview` record `ImVec2 cardTL = ImGui::GetCursorScreenPos();` before the header draws.

- [ ] **Step 2: Draw the animated border at the end.** Just before the function returns, when `g_LibEditMode`, draw a gold marching-ants rectangle around the whole card using four `DrawDottedLine` calls on the foreground draw list, between `cardTL` and the current cursor Y:

```cpp
if (g_LibEditMode) {
    ImVec2 br(cardTL.x + ImGui::GetContentRegionAvail().x, ImGui::GetCursorScreenPos().y);
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    ImU32 gold = IM_COL32(227,196,122,255);
    DrawDottedLine(fg, ImVec2(cardTL.x,cardTL.y), ImVec2(br.x,cardTL.y), gold, 2.0f, 6.0f, 4.0f);
    DrawDottedLine(fg, ImVec2(br.x,cardTL.y),     ImVec2(br.x,br.y),     gold, 2.0f, 6.0f, 4.0f);
    DrawDottedLine(fg, ImVec2(br.x,br.y),         ImVec2(cardTL.x,br.y), gold, 2.0f, 6.0f, 4.0f);
    DrawDottedLine(fg, ImVec2(cardTL.x,br.y),     ImVec2(cardTL.x,cardTL.y), gold, 2.0f, 6.0f, 4.0f);
}
```

- [ ] **Step 3: "EDITING" badge in the header.** When `g_LibEditMode`, draw an amber pill near the name field (to the right of the title input) via the draw list: a filled rounded rect `IM_COL32(120,90,30,220)` with text "● EDITING" in `IM_COL32(245,210,120,255)`.

- [ ] **Step 4: Build + in-game:** edit mode shows the animated gold frame + badge; view mode shows neither. Confirm the frame tracks the card height across professions (Rev/Ranger extra rows).

- [ ] **Step 5: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Build editor: edit-mode visual cues (animated border + EDITING badge)"
```

---

### Task 8: In-game validation pass against invariants

**Files:** none (validation only).

- [ ] **Step 1: OOTB/view-mode parity** — a build in view mode looks identical to before this work (banner, connectors, skills, gear, Share/Edit/Delete).
- [ ] **Step 2: Edit round-trips** — for a core build, an elite build, a Revenant (legends), and a Ranger (pets): enter edit, change a trait/skill/spec/gear/name/mode, Save, reselect → persisted; repeat but Cancel → reverted.
- [ ] **Step 3: Dirty/Cancel** — Cancel with no change closes silently; Cancel with a change prompts; Discard reverts; Keep editing stays.
- [ ] **Step 4: Edge cases** — empty spec slots, switching selected build mid-edit (should not corrupt draft), placeholder-trait imported builds.
- [ ] **Step 5: Update handover.md** with the finished state and any caveats found; commit.

```bash
git add handover.md
git commit -m "Build editor: finish in-place editing; update handover"
```

## Self-Review

- **Spec coverage:** in-place card (T2,T5), marching-ants reroute (T2), spec banner click (T3), header chrome in place (T5), skill grid restyle (T4), dirty/Save/Cancel (T1,T5), gear Cancel-safe (T6), visual cues (T7), view-mode parity invariant (every task Step verifies). All design sections mapped.
- **Placeholder scan:** no TBDs; the one omission (skill-category grouping) is explicitly resolved with rationale, not deferred work pretending to be done.
- **Type consistency:** `EnterBuildEditMode`, `MarkEditDirty`, `g_EditDirty` used consistently across tasks; picker globals match existing names (`g_SpecPickerSlot`, `g_SkillPickerSlot`, `g_LegendPickerSlot`, `g_PetPickerSlot`); `ReplaceSavedBuildDefinition`/`SetSavedBuildGameMode`/`UpdateSavedBuild` match existing GW2API signatures.
- **Open risk:** Task 6 changes gear-dialog data ownership — highest-risk task; flagged for user sign-off, with a documented fallback.
