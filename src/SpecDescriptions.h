#pragma once
#include <string>
#include <unordered_map>

namespace AlterEgo {

struct SpecDescription {
    const char* flavor;
    const char* description;
};

inline const SpecDescription* GetSpecDescription(const std::string& name) {
    static const std::unordered_map<std::string, SpecDescription> s_descs = {
        // ===== Guardian =====
        {"Zeal", {
            "Behold, the symbol of your defeat!",
            "Zeal utilizes the power of symbols and spirit weapons to punish your enemies. It strengthens the power of your justice virtue as well as your ability to apply vulnerability to foes."
        }},
        {"Radiance", {
            "Burn away the unworthy.",
            "Radiance improves your ability to burn and blind your foes, as well as improving your critical-hit capability. It grants bonuses while you are under the effects of resolution and strengthens the effects of your signets."
        }},
        {"Valor", {
            "My conviction shields me.",
            "Valor focuses heavily on blocking and meditation-based abilities, relying on aegis and various boon enhancements. Block attacks and protect your allies, enduring any onslaught that your foes may present."
        }},
        {"Honor", {
            "On my honor, I will protect you.",
            "Honor improves your ability to assist allies through healing, cleansing, and revival. Many of its abilities affect multiple allies at once and have large areas of effect."
        }},
        {"Virtues", {
            "Justice, resolve, and courage carry me forward.",
            "A guardian's virtue improves the effects of their justice, resolve, and courage-based skills, granting them additional effects and bonuses."
        }},
        {"Dragonhunter", {
            "I will set my trap and wait for them to come.",
            "The dragon hunter is a ranged, methodical hunter that uses various traps to ensnare their prey while picking them off with arrows of light. Their virtue skills become corporeal and more powerful in exchange for a longer casting time."
        }},
        {"Firebrand", {
            "Let the ancient magics of Vabbi burn away our foes.",
            "Firebrands harness the power of ancient Vabbian tomes to burn away impurities and conquer their foes. Each tome contains raw, powerful magic and must be used with patience and care."
        }},
        {"Willbender", {
            "I will silence the threat before it can fester.",
            "Willbenders are protectors of the Canthan throne and fated guardians to the Weh no Su. Their sword arm is as strong as their loyalty \u2014 unflinching and ruthless. They are known for a swift hand and unmatched swordplay, striking before their foe has time to react."
        }},
        {"Luminary", {
            "The light binds us!",
            "The Iron and Flame Legions worked in collaboration to harness radiant flame magic, creating a brazier for guardians to carry into battle. This powerful torch gives the guardian who wields it access to protective magic, making luminaries symbols of hope among the charr."
        }},
        // ===== Revenant =====
        {"Corruption", {
            "Writhe in anguish!",
            "Channel the immense and dark powers of Mallyx the Unyielding, the legendary demon. You specialize in applying conditions to your foes and inflicting torment on all who stand in your way. You are less susceptible to condition damage."
        }},
        {"Retribution", {
            "Ironhammer is ready!",
            "Channel the power of the legendary dwarf Jalis Ironhammer, and stand resolute against your enemies. This specialization offers a multitude of defensive abilities, strengthening your endurance and mitigating damage."
        }},
        {"Salvation", {
            "Live well, fully, and waste nothing.",
            "Salvation grants the power of the legendary centaur Ventari to heal your allies and protect them from harm. Create healing orbs to rejuvenate teammates while protecting yourself with additional boons."
        }},
        {"Invocation", {
            "I call upon the great legends of the Mists...",
            "Invocation grants bonuses when switching between legends, giving you access to the fury boon. Enhancements to fury give additional damage and an increased critical-hit chance."
        }},
        {"Devastation", {
            "I will turn this world to waste!",
            "Channel the power of the legendary assassin Shiro Tagachi, ripping through your foes and making them vulnerable. Gain quickness and might, punishing your enemies while keeping yourself alive in the heat of battle."
        }},
        {"Herald", {
            "You have a destiny to fulfill.",
            "The herald invokes the legendary dragon Glint, gaining abilities that allow them to channel restorative energies in order to bolster their allies. These abilities can be maintained indefinitely as long as they have sufficient energy, and they can be stacked to gain multiple effects."
        }},
        {"Renegade", {
            "Legions, fall in!",
            "The renegade channels the power of the legendary charr Kalla Scorchrazor, gaining abilities that help them control the battlefield. Summon warband members from the Mists to bolster your allies or crush your enemies."
        }},
        {"Vindicator", {
            "Once, we fought each other... Now we unite against a common enemy.",
            "Channel the Luxon and Kurzick heroes Archemorus and Saint Viktor to assault your enemies and protect your allies. Instead of dodging, you leap high into the air, crashing down to devastate your foes."
        }},
        {"Conduit", {
            "Hero of Tyria. I fight with you.",
            "Connect with a mysterious entity born of the Mists, Razah, and act as their conduit in battle. Gain Cosmic Wisdom and take on the stats and abilities of other revenant legends, amplified by Razah's power."
        }},
        // ===== Warrior =====
        {"Strength", {
            "Hit hard, and if that doesn't work, just hit harder.",
            "Strength grants might and improves the damage of several skills. Additionally, it improves the effectiveness of skills that disrupt the enemy."
        }},
        {"Arms", {
            "Cut them down where they stand.",
            "Arms specialization improves your ability to inflict bleeding on foes as well as your critical hit chance. It also grants access to the fury boon and methods to set up foes for more damage."
        }},
        {"Defense", {
            "The best offense is a good defense.",
            "Defense focuses on enduring damage and recovering from injury. Shrug off debilitating conditions and survive the assault from your foes."
        }},
        {"Tactics", {
            "The right strategy will see us through the day.",
            "Tactics utilizes the Soldier's Focus ability to bolster allies and manipulate the flow of battle. Improve the effectiveness of skills that affect allies to turn any fight in your favor."
        }},
        {"Discipline", {
            "Focus your anger in one burst.",
            "Discipline reduces the time between swapping weapons and improves adrenaline gain in order to allow more frequent use of burst skills."
        }},
        {"Berserker", {
            "Rage. Fire. Destruction!",
            "Embrace your primal side and enter berserk mode to become an avatar of fire and destruction. Deal high damage but be careful that your recklessness doesn't result in an early grave."
        }},
        {"Spellbreaker", {
            "I counter your magics!",
            "Act with precision and intent to strike at the very weave of magic to cut down your foe's enchantments while disrupting their ability to cast spells. Utilize the Full Counter skill to absorb attacks and retaliate with devastating force."
        }},
        {"Bladesworn", {
            "Concentrate and execute.",
            "Dedicated to the art of wielding the dangerous Canthan gunsaber, Bladesworn employ the meditative Dragon Trigger technique to strike down their foes with devastating slashes and point-blank blasts."
        }},
        {"Paragon", {
            "We will never yield!",
            "Paragons are the protectors of Elona, inspiring and rallying allies to their cause. Alongside their martial prowess, they channel their adrenaline towards maintaining chants that bolster allies and demoralize enemies."
        }},
        // ===== Engineer =====
        {"Explosives", {
            "I'd duck if I were you.",
            "Specializing in explosives improves the power of your explosion skills and grenade kit, giving them bonuses and additional enhanced effects. It allows you to make an Explosive Entrance on your enemies, adding an area effect to your first attack."
        }},
        {"Firearms", {
            "Pick 'em off, one at a time.",
            "Specializing in firearms improves your critical-hit efficiency and enhances your bleeding and burning potency. It grants various condition damage bonuses and strengthens your flamethrower."
        }},
        {"Inventions", {
            "Just a couple more twists here and...done!",
            "Specializing in inventions grants access to protective boons and several defensive gadgets that'll help you out of...sticky situations. From smoke bombs to proximity mines and turrets, this specialization offers a plethora of creative options for dominating the battlefield."
        }},
        {"Alchemy", {
            "This one'll knock a little life into you.",
            "Engineers that are adept in alchemy have access to various elixirs that either grant various boons or cure ailments. Use your elixirs to heal yourself on the fly or give you a little boost in combat."
        }},
        {"Tools", {
            "An engineer is as good as their tool belt, I'd say.",
            "Specializing in tools focuses on enhancing the use of tool belt skills and utility kits, granting them added bonuses and effects. It also enhances your dodging mobility and endurance."
        }},
        {"Scrapper", {
            "I'll patch you right on up.",
            "Scrappers are support-oriented engineers with access to gyros and a plethora of other cleansing abilities. You can aid multiple allies in large-scale battles with massive area-of-effect skills, healing and providing superspeed and other necessary boons."
        }},
        {"Holosmith", {
            "Burnin' up!",
            "Use your Photon Forge to generate weapons of pure, blinding light, cauterizing your enemies \u2014 but be careful not to use these weapons for too long, or you'll overheat."
        }},
        {"Mechanist", {
            "All systems go. Mech cleared for launch!",
            "The mechanist is an innovator and master of jade tech, battling alongside a mech of their own creation. Customize your mech and use it to crush your enemies \u2014 together, you're unstoppable."
        }},
        {"Amalgam", {
            "Synchronize!",
            "Asuran research has led to the creation of a new ooze subspecies, the mercurial mold. When an amalgam is bonded to one of these molds, they can command their mold to take any form \u2014 from a blunt weapon to a body-tight shield."
        }},
        // ===== Ranger =====
        {"Marksmanship", {
            "You can't outrun my arrow.",
            "Marksmanship augments and improves ranged combat, utilizing a powerful opening strike to debilitate enemies and pick them off before they can close the distance. Signets can also be enhanced."
        }},
        {"Skirmishing", {
            "Swift as a bird, bite like a serpent.",
            "Skirmishing improves your mobility in combat, granting various bonuses when you swap weapons and encouraging quick movement. It gives additional benefits when you evade attacks and enhances your traps."
        }},
        {"Wilderness Survival", {
            "In the wild...all you can rely on is yourself.",
            "Wilderness survival offers techniques to improve your defense in order to stay alive during intense combat. It grants both offensive and defensive condition augments depending on whether you are applying conditions or receiving them."
        }},
        {"Nature Magic", {
            "Nature, guide me...",
            "Nature magic focuses on the application of supportive and regenerative boons to yourself, your pet, and your allies. Your spirits are also enhanced, providing additional support."
        }},
        {"Beastmastery", {
            "We strike as one.",
            "Specializing in beast mastery enhances your overall synergy with your pet by strengthening their beast skill, improving on their abilities and granting various boons after successfully striking a foe."
        }},
        {"Druid", {
            "The stars, earth, and sky above will heal us.",
            "Druids utilize the power of nature and the stars above to heal their allies, using glyph skills to channel powerful boons and protective utilities. By generating your astral force, you can channel your Celestial Avatar form."
        }},
        {"Soulbeast", {
            "Two hearts fighting as one.",
            "Soulbeasts gain the ability to merge with their pet, channeling their power and the ability to use their beast skills. One with your pet, you are fierce in melee-focused combat."
        }},
        {"Untamed", {
            "Unleashed, unbound \u2014 the rage of the Echovald Forest lives in us.",
            "The Untamed share a primal bond with nature through their pet, juggling control of raw power back and forth to defeat foes \u2014 your pet uses debilitating nature-based skills to weaken enemies, then relinquishes that power to you for the final strike."
        }},
        {"Galeshot", {
            "I carry the wind.",
            "Archers from the Dominion of the Winds have learned to harness the powerful squall magic of their forests to weave a bow made of pure wind. That bow in hand, the galeshot manipulates the gusts around them to gracefully maneuver the battlefield and carry their arrows."
        }},
        // ===== Thief =====
        {"Deadly Arts", {
            "Would you like a taste of poison?",
            "Deadly arts focuses on the application and enhancement of the poison condition as well as increasing damage dealt from other sources. It also offers various improvements to stealing, which can further broaden offensive options."
        }},
        {"Critical Strikes", {
            "Strike when they're weak.",
            "Critical strikes improve your ability to critically hit the enemy. Whether it be increasing the damage of critical hits or improving access to the fury boon, this trait line offers multiple options and augments."
        }},
        {"Shadow Arts", {
            "Draped in shadow...",
            "The shadow arts specialization is focused around going into stealth and using stealth attacks. Increasing stealth duration and adding additional effects while in or exiting of stealth are the focus of this line."
        }},
        {"Acrobatics", {
            "Dancing through battle with a twirl of my blade.",
            "Acrobatics is focused primarily on dodging attacks and increasing your overall mobility. This line also offers a few general survivability options, such as self-healing and condition removal, depending on the situation."
        }},
        {"Trickery", {
            "Watch your step.",
            "Trickery offers new ways to manage initiative in combat as well as various augments to the Steal skill."
        }},
        {"Daredevil", {
            "Try keepin' up with this!",
            "The daredevil is a master of maneuverability and mobility. Gaining an additional endurance bar and the ability to customize their dodge roll, the daredevil has the tools to become a melee whirlwind of destruction."
        }},
        {"Deadeye", {
            "One shot, one kill.",
            "Take aim with your rifle and mark your enemies for assassination. The deadeye specializes in long-range combat and picking off enemies before they can get close. By marking your target, you gain increased damage against them as you wear them down before taking the final shot."
        }},
        {"Specter", {
            "Join me \u2014 the darkness is our ally.",
            "The Specter harnesses the power of shadow magic to debilitate enemies and preserve the health of allies. Enter Shadow Shroud to link with friends and fuel the destruction of your foes."
        }},
        // ===== Elementalist =====
        {"Fire", {
            "Raging flames, annihilate my foes.",
            "Fire-based magic focuses on raw power and spells, dealing severe damage to multiple foes at once. Attuning to fire improves your ability to inflict burning and apply might to yourself."
        }},
        {"Air", {
            "Swift as the wind, deadly as lightning.",
            "Air-based magic specializes in high single-target damage and improving your mobility. It enhances the power of your critical hits and grants bonus effects when you disable an enemy."
        }},
        {"Earth", {
            "I am a mountain, solid and enduring.",
            "Earth-based magic grants defensive benefits, reducing incoming damage and the severity of conditions. Attuning to earth gives you a superior advantage in combat, granting you abilities that cripple and immobilize your foes."
        }},
        {"Water", {
            "Soothing water heals all wounds.",
            "Water-based magic specializes in superior support abilities, healing allies and cleansing various ailments. Utilizing various water-based abilities also allows you to support your allies by chilling your foes and inflicting vulnerability."
        }},
        {"Arcane", {
            "The elements are at my command!",
            "The arcane specialization focuses on switching between elemental attunements, utilizing various benefits when jumping to a new element. Arcane abilities and traits also offer several ways to gain boons and additional boon duration."
        }},
        {"Tempest", {
            "My power, coming to a peak!",
            "Overload your attunements to become a powerful singularity of elemental power, shouting at your foes and calling upon your allies. The tempest specialization offers superior area-of-effect abilities in exchange for a slower attunement rotation."
        }},
        {"Weaver", {
            "The elements are twisting, turning \u2014 melding as one.",
            "The weaver gains the ability to combine two different elements and wield them simultaneously. Rapidly combine elements to create a fast and dynamic fighting style that dances in and out of combat."
        }},
        {"Catalyst", {
            "My power shapes and bends the world.",
            "Master cutting-edge Canthan technology to wield nature's forces with scientific precision. Imbue the elements into your jade-tech Sphere, creating wells of immense power."
        }},
        {"Evoker", {
            "Hello, friend.",
            "Evocation has been a veiled tradition among norn shamans for centuries, but now the Spirits of the Wild are encouraging them to share their practice \u2014 to help the Spirits heal. Thus, Evokers summon elementally attuned animal familiars to aid them in battle and join them on their ventures."
        }},
        // ===== Mesmer =====
        {"Domination", {
            "My mind is a dangerous place.",
            "Specializing in Domination gives the mesmer relentless prowess in battle, focusing on disrupting foes before they can strike. They chip away at their enemy's boons, inflicting vulnerability and weakening them."
        }},
        {"Dueling", {
            "Dancing through the battlefield, swift as a fleeting memory.",
            "Swift and ruthless, Dueling enhances your phantasm's damage output as well as your overall critical hit capability. Dodge your enemies with ease and gain additional bonuses after successfully evading attacks."
        }},
        {"Chaos", {
            "Where some see chaos, I see opportunity.",
            "Focusing heavily on disorienting their enemies at the height of battle, a mesmer's chaos magic enhances the application of boons \u2014 often at random \u2014 and other improvements to personal defense. It also provides bonuses to condition and boon duration."
        }},
        {"Inspiration", {
            "True beauty is an inspiration to all who behold.",
            "Inspiration magic focuses on healing yourself and nearby allies, as well as improving the potency of your phantasms. It also grants access to additional reflection abilities and strengthens your mantra and signet-based skills."
        }},
        {"Illusions", {
            "Or am I an illusion, too?",
            "Illusion magic specializes in the creation and enhancement of your clones and phantasms. It also improves the power of your shatter skills, granting a vicious and elusive advantage in battle."
        }},
        {"Chronomancer", {
            "And once more...",
            "Chronomancers manipulate the battlefield through the use of wells, creating brief ripples in time that give their allies various boons and slowing your enemies. They also have access to the powerful alacrity boon, reducing skill cooldowns."
        }},
        {"Mirage", {
            "A ripple in the sand, you see me no more.",
            "Cloaking themselves in rich desert sands, the mirage can execute skills and strike their foes while simultaneously dodging incoming attacks. You dance between your illusions, taking their place and deceiving your enemies."
        }},
        {"Virtuoso", {
            "I imagine blades, you bleed.",
            "The Virtuoso forgoes traditional illusions in favor of psychic daggers, wielded by the sheer power of the mind. Unleash a volley of sharp blades, then outwit your foes with a sharp mind."
        }},
        // ===== Necromancer =====
        {"Spite", {
            "You will feel my spite, deep in your bones.",
            "Your growing spite increases your outgoing damage by enhancing your ability to gain might and strengthening your signets. Strip boons from your enemies, leaving them defenseless against the onslaught."
        }},
        {"Curses", {
            "Your suffering fuels me.",
            "Specializing in curses improves the potency of your condition output and critical-hit efficiency. Increase the number of conditions you can apply, and send conditions applied to you right back to your foes."
        }},
        {"Death Magic", {
            "Death is only the beginning.",
            "Death magic focuses on the summoning and use of undead minions. As you create minions or inflict suffering on your foes, you can gain Death's Carapace, granting additional toughness and increasing power."
        }},
        {"Blood Magic", {
            "Your life for mine.",
            "Use the unrelenting power of blood magic to heal yourself by draining the life from your foes. Siphon dark energies into healing for your allies, allowing them to cheat death and live on."
        }},
        {"Soul Reaping", {
            "I will feast upon your soul.",
            "Gorge on the souls of your defeated enemies, improving the power of your Death Shroud and increasing your life force capability. Gain additional enhancements to your shroud to inflict further pain, stifling the life from your foes."
        }},
        {"Reaper", {
            "My scythe, it thirsts...",
            "The reaper is a harbinger of death, cleaving through multiple foes at once. Utilize your Reaper's Shroud, chilling your foes and gaining access to a spectral scythe to cut down all who stand in your way."
        }},
        {"Scourge", {
            "Sink into a sea of sand.",
            "The scourge harnesses the power of the desert to summon sand shades onto the battlefield. While they lose many of the protective benefits of death shroud, they control the battlefield by afflicting foes while bolstering allies and granting barrier."
        }},
        {"Harbinger", {
            "Volatile powers, all at my fingertips...",
            "The Harbinger is a gunslinger and an alchemist, empowering their pistol and utilizing elixirs imbued with dark energies. But strength comes at a cost, and the Harbinger must balance on a knife's edge between power and corruption."
        }},
    };

    auto it = s_descs.find(name);
    return (it != s_descs.end()) ? &it->second : nullptr;
}

} // namespace AlterEgo
