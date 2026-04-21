import { encodeAE2, decodeAE2 } from '../src/index.js';

let passed = 0;
let failed = 0;

function assert(cond, msg) {
  if (cond) { passed++; }
  else { failed++; console.error('FAIL:', msg); }
}

function assertEq(a, b, msg) {
  assert(a === b, `${msg}: expected ${JSON.stringify(b)}, got ${JSON.stringify(a)}`);
}

// ── Test 1: Minimal encode/decode (chat link only) ─────────────────────

{
  const chatLink = '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]';
  const code = encodeAE2({ chatLink });
  assert(code.startsWith('AE2:'), 'minimal: starts with AE2:');

  const decoded = decodeAE2(code);
  assertEq(decoded.chatLink, chatLink, 'minimal: chatLink roundtrip');
  assertEq(decoded.gameMode, 'PvE', 'minimal: default gameMode');
  assertEq(decoded.runeId, 0, 'minimal: no rune');
  assertEq(decoded.relicId, 0, 'minimal: no relic');
  assertEq(Object.keys(decoded.gear).length, 0, 'minimal: no gear');
}

// ── Test 2: Full encode/decode (gear + rune + relic + weapons) ─────────

{
  const input = {
    chatLink: '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]',
    gameMode: 'Raid',
    gear: {
      Helm:       { statId: 1077 },
      Shoulders:  { statId: 1077 },
      Coat:       { statId: 1077 },
      Gloves:     { statId: 1077 },
      Leggings:   { statId: 1077 },
      Boots:      { statId: 1077 },
      WeaponA1:   { statId: 1077, sigilId: 24615, weaponType: 'Greatsword', sigil2Id: 24868 },
      WeaponB1:   { statId: 1077, sigilId: 24618, weaponType: 'Scepter' },
      WeaponB2:   { statId: 1077, sigilId: 24615, weaponType: 'Focus' },
      Backpack:   { statId: 1077 },
      Accessory1: { statId: 1077 },
      Accessory2: { statId: 1077 },
      Amulet:     { statId: 1077 },
      Ring1:      { statId: 1077 },
      Ring2:      { statId: 1077 },
    },
    runeId: 24836,
    relicId: 100916,
  };

  const code = encodeAE2(input);
  assert(code.startsWith('AE2:'), 'full: starts with AE2:');
  assert(code.length < 200, `full: fits in chat (${code.length} chars)`);

  const d = decodeAE2(code);
  assertEq(d.chatLink, input.chatLink, 'full: chatLink');
  assertEq(d.gameMode, 'Raid', 'full: gameMode');
  assertEq(d.runeId, 24836, 'full: runeId');
  assertEq(d.relicId, 100916, 'full: relicId');

  // Armor stats
  for (const slot of ['Helm', 'Shoulders', 'Coat', 'Gloves', 'Leggings', 'Boots']) {
    assertEq(d.gear[slot]?.statId, 1077, `full: ${slot} statId`);
  }

  // Trinkets
  for (const slot of ['Backpack', 'Accessory1', 'Accessory2', 'Amulet', 'Ring1', 'Ring2']) {
    assertEq(d.gear[slot]?.statId, 1077, `full: ${slot} statId`);
  }

  // Weapons
  assertEq(d.gear.WeaponA1?.statId, 1077, 'full: WeaponA1 statId');
  assertEq(d.gear.WeaponA1?.sigilId, 24615, 'full: WeaponA1 sigilId');
  assertEq(d.gear.WeaponA1?.weaponType, 'Greatsword', 'full: WeaponA1 weaponType');
  assertEq(d.gear.WeaponA1?.sigil2Id, 24868, 'full: WeaponA1 sigil2Id');

  assertEq(d.gear.WeaponB1?.sigilId, 24618, 'full: WeaponB1 sigilId');
  assertEq(d.gear.WeaponB1?.weaponType, 'Scepter', 'full: WeaponB1 weaponType');
  assertEq(d.gear.WeaponB1?.sigil2Id, 0, 'full: WeaponB1 sigil2Id');

  assertEq(d.gear.WeaponB2?.sigilId, 24615, 'full: WeaponB2 sigilId');
  assertEq(d.gear.WeaponB2?.weaponType, 'Focus', 'full: WeaponB2 weaponType');
}

// ── Test 3: All game modes ─────────────────────────────────────────────

{
  const link = '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]';
  for (const mode of ['PvE', 'WvW', 'PvP', 'Raid', 'Fractal', 'Other']) {
    const code = encodeAE2({ chatLink: link, gameMode: mode });
    const d = decodeAE2(code);
    assertEq(d.gameMode, mode, `gameMode: ${mode}`);
  }
}

// ── Test 4: All weapon types ───────────────────────────────────────────

{
  const link = '[&DQEeHjElPy5LFwAAhgAAAEgBAACGAAAALRcAAAAAAAAAAAAAAAAAAAAAAAA=]';
  const weapons = [
    'Axe', 'Dagger', 'Focus', 'Greatsword', 'Hammer', 'Longbow', 'Mace',
    'Pistol', 'Rifle', 'Scepter', 'Shield', 'Shortbow', 'Spear', 'Staff',
    'Sword', 'Torch', 'Warhorn',
  ];
  for (const wt of weapons) {
    const code = encodeAE2({
      chatLink: link,
      gear: { WeaponA1: { statId: 1077, weaponType: wt } },
    });
    const d = decodeAE2(code);
    assertEq(d.gear.WeaponA1?.weaponType, wt, `weaponType: ${wt}`);
  }
}

// ── Test 5: Error handling ─────────────────────────────────────────────

{
  let threw = false;
  try { decodeAE2('not-ae2'); } catch { threw = true; }
  assert(threw, 'error: rejects non-AE2 input');

  threw = false;
  try { encodeAE2({}); } catch { threw = true; }
  assert(threw, 'error: rejects missing chatLink');

  threw = false;
  try { decodeAE2('AE2:AA=='); } catch { threw = true; }
  assert(threw, 'error: rejects unsupported version');
}

// ── Summary ────────────────────────────────────────────────────────────

console.log(`\n${passed} passed, ${failed} failed`);
if (failed > 0) process.exit(1);
