# Theming Consistency + H&S Empty State — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring stragglers into the gold-on-dark vocabulary used elsewhere, and replace the terse "H&S not detected" text with an inviting centered card.

**Architecture:** Add four file-static helpers near `RenderSectionHeader` in [src/dllmain.cpp](src/dllmain.cpp) (`RenderGoldButton`, `RenderChipButton`, `RenderThemedCombo`, `RenderEmptyCard`). Walk every tab swapping plain `ImGui::Button` / `ImGui::Combo` calls for the helpers. Replace the single-line "H&S not detected" text above the tab bar with the empty card when status is `Unavailable`, suppressing the tab bar in that case so the card owns the content region.

**Tech Stack:** ImGui 1.80 (`lib/imgui/`), Nexus addon API v6, MinGW cross-compile. No test suite — verification is `cd build && make -j$(nproc)` after each task plus visual review in-game.

**Design source:** [plans/2026-05-22-theming-and-empty-states-design.md](plans/2026-05-22-theming-and-empty-states-design.md)

**Important constraints from [CLAUDE.md](CLAUDE.md):**
- Build the DLL after every code change and confirm clean compile.
- Do not push to GitHub or tag releases.
- Do not add `Co-Authored-By` trailers.
- User edits files between turns — always re-read before editing.

---

## Task 1: Add `RenderGoldButton` helper

**Files:**
- Modify: [src/dllmain.cpp](src/dllmain.cpp) — add helper near line 10068 (immediately after `RenderSectionHeader`).
- Also need a forward declaration if any earlier code calls it. Check [src/AddonShared.h](src/AddonShared.h) for the existing `RenderSectionHeader` decl pattern and follow it.

- [ ] **Step 1: Read current state**

Read [src/dllmain.cpp:10030-10068](src/dllmain.cpp#L10030-L10068) to confirm `RenderSectionHeader` signature is still `void RenderSectionHeader(const char* label, ImVec4 color, const char* suffix)`. Read [src/AddonShared.h](src/AddonShared.h) to see where `RenderSectionHeader` is declared.

- [ ] **Step 2: Add helper implementation**

Insert directly after `RenderSectionHeader`'s closing `}` at line 10068. The button matches the gold-bar vocabulary: dark fill, gold border (2px), gold text, brighter on hover.

```cpp
// Gold-trimmed button — primary-action vocabulary matching RenderSectionHeader.
// size = ImVec2(0,0) auto-sizes to label; pass explicit size for fixed-width rows.
// Returns true on click.
bool RenderGoldButton(const char* label, ImVec2 size = ImVec2(0, 0)) {
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.14f, 0.06f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.22f, 0.10f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.36f, 0.28f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0.70f, 0.58f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.95f, 0.85f, 0.55f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    return clicked;
}
```

- [ ] **Step 3: Add forward declaration**

If `RenderSectionHeader` is declared in [src/AddonShared.h](src/AddonShared.h), add `bool RenderGoldButton(const char* label, ImVec2 size = ImVec2(0, 0));` immediately below it using the same visibility (likely `namespace` scope or top-level). If it's a file-local static instead, make `RenderGoldButton` file-local `static bool ...` too — no header change needed.

- [ ] **Step 4: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile, no new warnings about unused functions (the helper is unused this task — that's fine; subsequent tasks consume it).

- [ ] **Step 5: Commit**

```bash
git add src/dllmain.cpp src/AddonShared.h
git commit -m "Add RenderGoldButton helper for themed primary actions"
```

---

## Task 2: Add `RenderChipButton` helper

**Files:**
- Modify: [src/dllmain.cpp](src/dllmain.cpp) — add helper directly after `RenderGoldButton`.

- [ ] **Step 1: Add helper implementation**

The chip vocabulary already exists inline at multiple call sites (mode chips at lines 8086-8111, filter chips elsewhere). This consolidates it.

```cpp
// Small chip-style button — secondary action vocabulary.
// active: highlighted (gold tint) when true.
// accentColor: optional accent (e.g. profession colour). Pass ImVec4(0,0,0,0) for default gold.
// Returns true on click.
bool RenderChipButton(const char* label, bool active, ImVec4 accentColor = ImVec4(0, 0, 0, 0)) {
    bool hasAccent = (accentColor.w > 0.01f);
    ImVec4 base    = hasAccent ? ImVec4(accentColor.x * 0.30f, accentColor.y * 0.30f, accentColor.z * 0.30f, 0.55f)
                               : ImVec4(0.15f, 0.13f, 0.09f, 0.55f);
    ImVec4 hover   = hasAccent ? ImVec4(accentColor.x * 0.45f, accentColor.y * 0.45f, accentColor.z * 0.45f, 0.75f)
                               : ImVec4(0.25f, 0.20f, 0.10f, 0.75f);
    ImVec4 actVar  = hasAccent ? ImVec4(accentColor.x * 0.55f, accentColor.y * 0.55f, accentColor.z * 0.55f, 0.90f)
                               : ImVec4(0.50f, 0.40f, 0.18f, 0.80f);
    ImVec4 text    = active    ? ImVec4(0.95f, 0.85f, 0.55f, 1.00f)
                               : ImVec4(0.78f, 0.76f, 0.66f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_Button,        active ? actVar : base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  actVar);
    ImGui::PushStyleColor(ImGuiCol_Text,          text);
    bool clicked = ImGui::SmallButton(label);
    ImGui::PopStyleColor(4);
    return clicked;
}
```

- [ ] **Step 2: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 3: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Add RenderChipButton helper consolidating chip vocabulary"
```

---

## Task 3: Add `RenderThemedCombo` helper

**Files:**
- Modify: [src/dllmain.cpp](src/dllmain.cpp) — add helper directly after `RenderChipButton`.

- [ ] **Step 1: Add helper implementation**

This wraps `BeginCombo`/`EndCombo` with the gold-border + dark-fill styling. It uses a callback-based API so the caller can `Selectable` each item with whatever logic they want.

```cpp
// Gold-bordered combo box. Internally pushes themed colors before BeginCombo and pops them after EndCombo.
// `current` is the label shown when collapsed.
// `width` <= 0 means "let ImGui decide" (uses SetNextItemWidth caller-side instead).
// Caller wraps content in lambda body, calling ImGui::Selectable etc.
//
// Usage:
//   if (RenderThemedCombo("##acct", currentLabel.c_str(), 180.0f)) {
//       if (ImGui::Selectable("All Accounts", filter.empty())) filter.clear();
//       ...
//       ImGui::EndCombo();
//   }
//
// Returns true when the combo is open (same contract as ImGui::BeginCombo).
// Caller is responsible for ImGui::EndCombo() when the function returns true.
bool RenderThemedCombo(const char* id, const char* current, float width = -1.0f) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.10f, 0.09f, 0.06f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.13f, 0.08f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.20f, 0.16f, 0.09f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(0.70f, 0.58f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.95f, 0.85f, 0.55f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg,        ImVec4(0.08f, 0.07f, 0.05f, 0.98f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    if (width > 0.0f) ImGui::SetNextItemWidth(width);
    bool open = ImGui::BeginCombo(id, current);
    // NOTE: pops happen in a small RAII-less pattern — caller must call RenderThemedComboEnd()
    // after EndCombo (or after immediately, if not open). To avoid that, we pop here regardless
    // since BeginCombo applies the style to both the button AND its popup; popping now is fine
    // because BeginCombo has already snapshotted the style for the popup.
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(6);
    return open;
}
```

- [ ] **Step 2: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 3: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Add RenderThemedCombo helper for gold-bordered dropdowns"
```

---

## Task 4: Add `RenderEmptyCard` helper

**Files:**
- Modify: [src/dllmain.cpp](src/dllmain.cpp) — add helper directly after `RenderThemedCombo`.

- [ ] **Step 1: Add helper implementation**

Centered card with optional icon, headline, body, and up to two action buttons. Sized to ~420×260 and centered in the current content region.

```cpp
// Centered themed empty-state / first-run card.
// iconTex:  optional texture (Nexus Texture*). Pass nullptr to skip the icon row.
// headline: bold title text.
// body:     multi-line body text (text wrapping handled internally).
// primaryLabel / primaryCB:   primary action button (gold). Pass nullptr label to omit.
// secondaryLabel / secondaryCB: secondary action button (chip). Pass nullptr label to omit.
void RenderEmptyCard(Texture* iconTex,
                     const char* headline,
                     const char* body,
                     const char* primaryLabel, std::function<void()> primaryCB,
                     const char* secondaryLabel, std::function<void()> secondaryCB) {
    const float cardW = 420.0f;
    const float cardH = 260.0f;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float offsetX = (avail.x - cardW) * 0.5f;
    float offsetY = (avail.y - cardH) * 0.5f;
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;

    ImGui::Dummy(ImVec2(0, offsetY));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    ImVec2 cardOrigin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Card fill + gold border.
    dl->AddRectFilled(cardOrigin, ImVec2(cardOrigin.x + cardW, cardOrigin.y + cardH),
                      IM_COL32(20, 17, 11, 235), 6.0f);
    dl->AddRect      (cardOrigin, ImVec2(cardOrigin.x + cardW, cardOrigin.y + cardH),
                      IM_COL32(178, 148, 51, 255), 6.0f, 0, 1.5f);

    ImGui::BeginChild("##empty_card_inner", ImVec2(cardW, cardH), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Dummy(ImVec2(0, 12));

    // Icon row.
    if (iconTex && iconTex->Resource) {
        float iconSize = 64.0f;
        ImGui::SetCursorPosX((cardW - iconSize) * 0.5f);
        ImGui::Image(iconTex->Resource, ImVec2(iconSize, iconSize));
        ImGui::Dummy(ImVec2(0, 8));
    }

    // Headline (centered).
    if (headline && *headline) {
        ImVec2 sz = ImGui::CalcTextSize(headline);
        ImGui::SetCursorPosX((cardW - sz.x) * 0.5f);
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.55f, 1.0f), "%s", headline);
        ImGui::Dummy(ImVec2(0, 6));
    }

    // Body (wrapped, centered horizontally via a child).
    if (body && *body) {
        float bodyW = cardW - 40.0f;
        ImGui::SetCursorPosX((cardW - bodyW) * 0.5f);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + bodyW);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.76f, 0.66f, 1.0f));
        ImGui::TextWrapped("%s", body);
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();
        ImGui::Dummy(ImVec2(0, 14));
    }

    // Button row (centered).
    bool hasPrimary   = primaryLabel   && *primaryLabel;
    bool hasSecondary = secondaryLabel && *secondaryLabel;
    if (hasPrimary || hasSecondary) {
        float pW = hasPrimary   ? ImGui::CalcTextSize(primaryLabel).x   + 24.0f : 0;
        float sW = hasSecondary ? ImGui::CalcTextSize(secondaryLabel).x + 16.0f : 0;
        float gap = (hasPrimary && hasSecondary) ? 10.0f : 0.0f;
        float totalW = pW + sW + gap;
        ImGui::SetCursorPosX((cardW - totalW) * 0.5f);

        if (hasPrimary) {
            if (RenderGoldButton(primaryLabel, ImVec2(pW, 0))) {
                if (primaryCB) primaryCB();
            }
            if (hasSecondary) ImGui::SameLine(0, gap);
        }
        if (hasSecondary) {
            if (RenderChipButton(secondaryLabel, false)) {
                if (secondaryCB) secondaryCB();
            }
        }
    }

    ImGui::EndChild();
}
```

- [ ] **Step 2: Verify `Texture*` type and `<functional>` include**

Grep [src/dllmain.cpp](src/dllmain.cpp) for `Texture*` to confirm it is the Nexus type. If `std::function` is not already used, add `#include <functional>` near the top of [src/dllmain.cpp](src/dllmain.cpp).

```bash
grep -n "Texture\*\|#include <functional>" src/dllmain.cpp | head -5
```

- [ ] **Step 3: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Add RenderEmptyCard helper for first-run / empty states"
```

---

## Task 5: Audit — build editor action buttons

**Files:**
- Modify: [src/dllmain.cpp:4109](src/dllmain.cpp#L4109) — `Copy Build Chat Link` button.
- Modify: [src/dllmain.cpp:4191](src/dllmain.cpp#L4191) — `Save to Library` button.

- [ ] **Step 1: Re-read both call sites**

Open [src/dllmain.cpp:4100-4120](src/dllmain.cpp#L4100-L4120) and [src/dllmain.cpp:4185-4200](src/dllmain.cpp#L4185-L4200) so you see the exact surrounding context (the `hasPalette` ternary on the label).

- [ ] **Step 2: Replace both `ImGui::Button` calls with `RenderGoldButton`**

The existing logic stays identical — only the rendering changes. Each line currently looks like:
```cpp
if (ImGui::Button(hasPalette ? "Copy Build Chat Link" : "Copy Build Chat Link (loading...)") && hasPalette) {
```
becomes:
```cpp
if (RenderGoldButton(hasPalette ? "Copy Build Chat Link" : "Copy Build Chat Link (loading...)") && hasPalette) {
```
Same transformation for the `Save to Library` button at line 4191.

- [ ] **Step 3: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Theme build editor action buttons (Copy Chat Link, Save to Library)"
```

---

## Task 6: Audit — build library import controls + chat status chip

**Files:**
- Modify: [src/dllmain.cpp:7488](src/dllmain.cpp#L7488) — `+ Import Build` / `Cancel Import` toggle button.
- Modify: [src/dllmain.cpp:7497-7508](src/dllmain.cpp#L7497-L7508) — Chat addon status text.
- Modify: [src/dllmain.cpp:7540](src/dllmain.cpp#L7540) — `Import` button inside the import panel.
- Modify: [src/dllmain.cpp:8124](src/dllmain.cpp#L8124) — `Done` button in edit mode.

- [ ] **Step 1: Re-read all four sites**

Read [src/dllmain.cpp:7484-7545](src/dllmain.cpp#L7484-L7545) and [src/dllmain.cpp:8120-8135](src/dllmain.cpp#L8120-L8135).

- [ ] **Step 2: Replace primary action buttons with `RenderGoldButton`**

- Line 7488: `ImGui::Button(g_LibShowImport ? "Cancel Import" : "+ Import Build")` → `RenderGoldButton(...)` with same label expression.
- Line 7540: `ImGui::Button("Import")` → `RenderGoldButton("Import")`.

- [ ] **Step 3: Replace edit-mode `Done` with `RenderChipButton`**

Line 8124: `ImGui::SmallButton("Done##edit")` → `RenderChipButton("Done##edit", false)`. The `Cancel Import` already covered above is primary action — `Done` is secondary.

- [ ] **Step 4: Replace chat status text with status chip**

Lines 7498-7508 currently render either a green "Chat: Connected" or red "Chat: Not detected" text. Replace with a small chip:
- Connected:    draw a 6px green dot via `ImDrawList::AddCircleFilled` followed by `ImGui::TextColored(...,"Chat: Connected")` in a soft palette — or just `RenderChipButton("● Chat", true)` with a green accent. Simplest path that stays consistent: use `RenderChipButton` with a green accent when connected, red accent when not, no click handler (call it but discard the return).

```cpp
ImVec4 green(0.35f, 0.82f, 0.35f, 1.0f);
ImVec4 red  (1.00f, 0.40f, 0.40f, 1.0f);
ImGui::SameLine();
(void)RenderChipButton(g_ChatAddonConnected ? "● Chat: Connected" : "● Chat: Not detected",
                       /*active=*/g_ChatAddonConnected,
                       g_ChatAddonConnected ? green : red);
if (!g_ChatAddonConnected && ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::Text("Install 'Events: Chat' from the Nexus addon library");
    ImGui::Text("to import builds shared in GW2 chat.");
    ImGui::EndTooltip();
}
```

- [ ] **Step 5: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Theme build library import controls and chat status chip"
```

---

## Task 7: Audit — account selector → themed combo

**Files:**
- Modify: [src/dllmain.cpp:11716](src/dllmain.cpp#L11716) — `BeginCombo("##AccountSelect", ...)`.

- [ ] **Step 1: Re-read the call site**

Read [src/dllmain.cpp:11700-11760](src/dllmain.cpp#L11700-L11760) to see the full combo body. There are `Selectable` calls inside.

- [ ] **Step 2: Replace `BeginCombo` with `RenderThemedCombo`**

The existing code:
```cpp
ImGui::SetNextItemWidth(comboWidth);
if (ImGui::BeginCombo("##AccountSelect", currentLabel.c_str())) {
    ... selectables ...
    ImGui::EndCombo();
}
```
becomes:
```cpp
if (RenderThemedCombo("##AccountSelect", currentLabel.c_str(), comboWidth)) {
    ... selectables ...
    ImGui::EndCombo();
}
```
Drop the now-redundant `SetNextItemWidth` line — the helper handles it.

- [ ] **Step 3: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Theme account selector combo with gold border"
```

---

## Task 8: Audit — Options tab full pass

**Files:**
- Modify: [src/dllmain.cpp:12810-12900](src/dllmain.cpp#L12810-L12900) and surrounding area.

- [ ] **Step 1: Re-read the entire Options tab body**

Find the Options tab boundary: `grep -n "BeginTabItem.*Options\|EndTabItem" src/dllmain.cpp | head -10` and read the full range. Likely starts a few hundred lines before line 12810.

- [ ] **Step 2: Replace section labels with `RenderSectionHeader`**

For each `ImGui::Text("UI Settings:")` / `"Chat Build Detection:"` / `"Data Source: Hoard & Seek"`-style label, replace with:
```cpp
RenderSectionHeader("UI Settings", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
```
Use the gold colour `(0.70, 0.58, 0.20, 1)` to match every other tab's section headers.

Drop the `ImGui::Separator()` lines that immediately precede or follow these labels — the section header already provides visual separation.

- [ ] **Step 3: Replace plain buttons with helpers**

In this region:
- `ImGui::SmallButton("Buy me a coffee!")` → `RenderChipButton("☕ Buy me a coffee!", false)` (line ~12811).
- `ImGui::SmallButton("Retry Connection")` (line 12840) → `RenderGoldButton("Retry Connection")`.
- `ImGui::Button("Reset Toast Position")` (line 12871) → `RenderChipButton("Reset Toast Position", false)`.

Any other `ImGui::Button` / `ImGui::SmallButton` inside the Options tab gets the same treatment: primary actions (retries, applies) → `RenderGoldButton`, secondary (links, resets, info) → `RenderChipButton`.

- [ ] **Step 4: H&S status block — keep inline, do not replace with empty card**

The empty card lives in the global top area (Task 10). The Options tab's H&S status block stays as informational text since it's already on a settings screen — only the buttons and section labels get themed.

- [ ] **Step 5: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 6: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Theme Options tab: gold section headers, helper-based buttons"
```

---

## Task 9: Audit — relay popup buttons

**Files:**
- Modify: [src/dllmain.cpp:8179-8201](src/dllmain.cpp#L8179-L8201) — `Copy Code`, `Done`, `OK` buttons in relay popups.

- [ ] **Step 1: Re-read the popup blocks**

Read [src/dllmain.cpp:8138-8202](src/dllmain.cpp#L8138-L8202).

- [ ] **Step 2: Replace buttons**

- `ImGui::Button("Copy Code", ImVec2(100, 0))` (line 8179) → `RenderGoldButton("Copy Code", ImVec2(100, 0))`.
- `ImGui::Button("Done", ImVec2(80, 0))` (line 8184) → `RenderChipButton("Done", false)`. Drop the explicit `ImVec2(80, 0)` size — chips are auto-sized.
- `ImGui::Button("OK", ImVec2(80, 0))` (line 8196) → `RenderChipButton("OK", false)`.

- [ ] **Step 3: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 4: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Theme relay popup buttons (Copy Code / Done / OK)"
```

---

## Task 10: H&S empty card — replace top-of-window warning

**Files:**
- Modify: [src/dllmain.cpp:11668-11689](src/dllmain.cpp#L11668-L11689) — `HoardStatus::Unavailable` branch.
- Likely needs to early-return from the addon render function before the tab bar draws, when status is `Unavailable`. Confirm the surrounding function structure first.

- [ ] **Step 1: Map the surrounding render function**

Read [src/dllmain.cpp:11650-11710](src/dllmain.cpp#L11650-L11710) to see how the warning currently sits above the tab bar. Identify where the tab bar (`BeginTabBar`) is called relative to the warning block.

- [ ] **Step 2: Carve out the Unavailable path**

When `hoardStatus == HoardStatus::Unavailable`, render the empty card and **skip the tab bar** so the card owns the content area. Other statuses (`Unknown`, `PermPending`, `PermDenied`) keep their current inline warning — they're transient states. Only `Unavailable` triggers the full card.

Pseudocode (adapt to the real surrounding control flow):
```cpp
if (hoardStatus == AlterEgo::HoardStatus::Unavailable) {
    // Pull the Nexus library URL — used for the secondary action.
    static const char* NEXUS_LIBRARY_URL = "https://raidcore.gg/Nexus";

    RenderEmptyCard(
        /*iconTex=*/ TEX_ICON,   // existing addon icon texture — verify exact symbol name in this file
        /*headline=*/ "Hoard & Seek is required",
        /*body=*/ "Alter Ego reads your characters, equipment, and inventory through the Hoard & Seek companion addon. Install it from the Nexus addon library to get started.",
        /*primaryLabel=*/ "Retry Detection",
        /*primaryCB=*/ []() { AlterEgo::GW2API::PingHoard(); },
        /*secondaryLabel=*/ "Copy Library URL",
        /*secondaryCB=*/ []() {
            CopyToClipboard(std::string("https://raidcore.gg/Nexus"));
            if (APIDefs) APIDefs->GUI_SendAlert("Nexus library URL copied!");
        }
    );
    // Skip the tab bar entirely while H&S is missing.
    return;  // or `goto end_render;` — match the existing function structure
}
```

Replace the existing single-line `ImGui::TextColored(..., "H&S not detected. Install Hoard & Seek.")` with the block above.

- [ ] **Step 3: Verify icon texture symbol**

Search for the icon used by the QuickAccess registration: `grep -n "TEX_ICON\|QuickAccess_Add" src/dllmain.cpp | head -5`. Use whatever symbol the QuickAccess registration uses (likely `TEX_ICON`).

If the texture lookup returns `nullptr` at first paint (textures load async), the card simply renders without the icon — `RenderEmptyCard` already null-checks. No defensive code needed.

- [ ] **Step 4: Build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile.

- [ ] **Step 5: Commit**

```bash
git add src/dllmain.cpp
git commit -m "Show themed empty card when Hoard & Seek is not detected"
```

---

## Task 11: Final visual review pass

**Files:** none — this is a checklist task before declaring done.

- [ ] **Step 1: Confirm clean build**

Run: `cd build && make -j$(nproc)`
Expected: clean compile, zero new warnings.

- [ ] **Step 2: Copy DLL into the game install (user action — ask the user)**

Stop here and tell the user the build is clean. They will copy `build/AlterEgo.dll` into `<GW2>/addons/AlterEgo.dll` and visually verify:

1. Every tab still renders.
2. Action buttons look themed (gold trim).
3. Account selector is gold-bordered.
4. Options tab section headers match other tabs.
5. With H&S off → centered card appears, tabs are hidden. With H&S on → tabs return.
6. Retry Detection on the card works once H&S is loaded.

- [ ] **Step 3: Update handover.md**

After user confirms visual review passes, edit [handover.md](handover.md) Session-3 section to record the theming pass + empty card and reference the design and plan documents.

- [ ] **Step 4: Final commit**

```bash
git add handover.md
git commit -m "handover: theming consistency pass + H&S empty card"
```

---

## Self-Review

**Spec coverage:**
- Helper layer: Tasks 1-4 cover `RenderGoldButton`, `RenderChipButton`, `RenderThemedCombo`, `RenderEmptyCard`. ✓
- Audit checklist: top window chrome (Task 7), build editor (Task 5), build library (Task 6), Options tab (Task 8), relay popups (Task 9). Achievements / Skinventory / Vault & Clears are spot-checks — covered by Task 11 visual review. ✓
- H&S empty card: Task 10. ✓
- Behaviour (auto-dismiss when H&S detected, Retry Detection button, library URL fallback): all in Task 10. ✓

**Placeholder scan:** none.

**Type consistency:** `RenderGoldButton` signature is consistent across Tasks 1, 5, 9, 10. `RenderChipButton` signature consistent across Tasks 2, 6, 8, 9, 10. `RenderEmptyCard` signature consistent between Task 4 and Task 10.

**One open dependency:** Task 10 references `TEX_ICON` — the actual symbol name is verified in Task 10 Step 3 by the implementing engineer, so no rewrite needed.
