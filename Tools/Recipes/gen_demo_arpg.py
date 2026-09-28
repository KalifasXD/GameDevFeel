"""Writes the Action/RPG demo kit recipes as JSON (GameFeelDev/DemoKits/ActionRPG/FR_ARPG_<Name>.json).

Uses the helpers of gen_library.py. Run: python gen_demo_arpg.py
"""
import json, os, shutil
from gen_library import step, track, mapping, parameter, color, vec, rot, curve, FLAT, FADE, RISE, SPIKE, BELL, SOUND

# The Action/RPG kit lives in the demo project, not in the plugin (D-089, D-091).
ROOT = r'B:\NewUE5Project\GameFeelDev\DemoKits\ActionRPG'
# Sounds only this kit uses sit next to it in the project; shared ones stay in the plugin.
KIT_SOUNDS = {'Body_Hit_Heavy', 'Guard_Break', 'Guard_Clang_Heavy', 'Guard_Clang_Light', 'Guard_Field', 'Parry_Ring',
              'Sword_Clash', 'Sword_Hit', 'Sword_Swing_Heavy', 'Sword_Swing_Light'}


def snd(name):
    return '/Game/FeelKitDemos/ActionRPG/Sounds/S_FK_{0}.S_FK_{0}'.format(name) if name in KIT_SOUNDS else SOUND.format(name)

HIT_FLASH = '/FeelKit/Samples/Materials/M_FK_HitFlash.M_FK_HitFlash'

# Swing strengths come from the Set Feel Value markers: 0.35 first swing, 0.65 second, 1.0 finisher and charged strike.
SWING = parameter('Swing', 0.35, 0.0, 1.0, 'How heavy the swing is: 0.35 first swing, 0.65 second, 1 finisher. Set by the Set Feel Value markers in the attack animations.', accumulator='Swing')
CHARGED = parameter('Charged', 0.0, 0.0, 1.0, '1 when the hit comes from the charged attack. Set by the Set Feel Value markers.', accumulator='Charged')
STREAK = parameter('Streak', 0.0, 0.0, 10.0, 'Hits in the current streak. Reads the Combo accumulator, which the player adds to on every hit.', accumulator='Combo')
CHARGE_LEVEL = parameter('ChargeLevel', 0.0, 0.0, 1.0, 'How long the charge has been held, 0 to 1. Each loop of the charge animation adds to it.', accumulator='ChargeLevel')

HEAVY_ONLY = [(0, 0), (0.5, 0), (0.55, 1), (1, 1)]
FINISHER_ONLY = [(0, 0), (0.9, 0), (0.95, 1), (1, 1)]
BY_SWING = [(0, 0.35), (1, 1)]
CHARGED_ONLY = [(0, 0), (0.5, 0), (0.55, 1), (1, 1)]
BEFORE_FINISHER = [(0, 1), (0.9, 1), (0.95, 0), (1, 0)]
RECIPES = []


def recipe(name, feeling, description, tracks, parameters=None):
    body = {
        'tracks': tracks,
        'cooldown': 0.0,
        'defaultIntensity': 1.0,
        'feeling': {'tagName': 'Feel.Feeling.' + feeling},
        'genres': {'gameplayTags': [{'tagName': 'Feel.Genre.Action'}]},
        'description': description,
    }
    if parameters:
        body['parameters'] = parameters
    RECIPES.append(('FR_ARPG_' + name, body))


recipe('Swing', 'Speed', 'Every swing, hit or miss: a whoosh and a slight lean of the camera into the swing.', [
    track(step('PlaySound', sound=snd('Sword_Swing_Light'), placement='AttachedToTarget', pitchVariation=0.08), 0.0, 0.35, FLAT,
          parameterMappings=[mapping('Swing', BEFORE_FINISHER)]),
    track(step('PlaySound', sound=snd('Sword_Swing_Heavy'), placement='AttachedToTarget', pitchVariation=0.05), 0.0, 0.45, FLAT,
          parameterMappings=[mapping('Swing', FINISHER_ONLY)]),
    # A slight lean into the swing. This and the hit's camera kick and shake below are the values the user approved on
    # 2026-09-19; every later retune of them was rejected (I-046), so they stay as they are.
    track(step('CameraPunch', locationPunch=vec(4, 0, 0), rotationPunch=rot(-0.6, 0, 0), frequency=6.0, damping=8.0), 0.0, 0.3,
          parameterMappings=[mapping('Swing', [(0, 0.4), (1, 1)])]),
], parameters=[SWING])

recipe('ChargePulse', 'Power', 'One loop of holding a charged attack: a hum, a shake and a rumble that grow the longer it is held.', [
    track(step('PlaySound', sound=snd('Charge_Loop'), placement='AttachedToTarget', bStopAtTrackEnd=True, fadeOutTime=0.1), 0.0, 0.47, FLAT,
          parameterMappings=[mapping('ChargeLevel', [(0, 0.4), (1, 1)])]),
    track(step('ProceduralShake', frequency=24.0, rotationAmplitude=rot(0.4, 0.4, 0.2), locationAmplitude=vec(0, 1, 1)), 0.0, 0.47, FLAT,
          parameterMappings=[mapping('ChargeLevel', [(0, 0.2), (1, 1)])]),
    track(step('ForceFeedbackCurve', leftLarge=0.15, rightLarge=0.15, leftSmall=0.5, rightSmall=0.5, shape='Smooth'), 0.0, 0.47, FLAT,
          parameterMappings=[mapping('ChargeLevel', [(0, 0.2), (1, 1)])]),
    track(step('VignettePulse', vignetteIntensity=0.5, shape='Smooth'), 0.0, 0.47, FLAT,
          parameterMappings=[mapping('ChargeLevel', [(0, 0), (1, 1)])]),
], parameters=[CHARGE_LEVEL])

SPARKS = '/Game/Variant_Combat/VFX/NS_Damage.NS_Damage'


def sparks(scale, **extra):
    """The template's own spark effect, bigger, at the hit location (FeelNiagara)."""
    return {
        'step': {'_ClassName': '/Script/FeelNiagara.FeelStep_SpawnParticle', 'system': SPARKS, 'scale': vec(scale, scale, scale), 'bDeactivateWithTrack': False},
        'startTime': 0.0, 'duration': 0.05, 'channel': {'tagName': 'Feel.Spawn'}, 'intensityCurve': FLAT, **extra,
    }


recipe('HitLanded', 'Impact', 'The player\'s hit connects. Light, heavy and finisher from one recipe, driven by the Swing marker; the charged strike adds a crit on top.', [
    # Every hit: the whole world freezes for a moment; heavier swings freeze longer.
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.06, FLAT),
    track(step('GlobalHitstop', timeDilation=0.02), 0.06, 0.04, FLAT, parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    track(step('GlobalHitstop', timeDilation=0.02), 0.10, 0.05, FLAT, parameterMappings=[mapping('Swing', FINISHER_ONLY)]),
    # The camera kicks and shakes: a springy jolt and a fast rattle, both playing on through the freeze.
    track(step('CameraPunch', locationPunch=vec(-20, 0, -7), rotationPunch=rot(-5.5, 0, 0), frequency=9.0, damping=7.0), 0.0, 0.4,
          parameterMappings=[mapping('Swing', [(0, 0.6), (1, 1.2)])]),
    track(step('ProceduralShake', frequency=24.0, rotationAmplitude=rot(3.0, 3.0, 1.6), locationAmplitude=vec(0, 6, 6)), 0.0, 0.3,
          parameterMappings=[mapping('Swing', [(0, 0.5), (1, 1.2)])]),
    track(step('FOVKick', fieldOfViewKick=10.0, frequency=7.0, damping=7.0), 0.0, 0.4, parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    # Heavy hits: a color fringe. No full-screen flash: the user found it odd on the heavy strikes (I-046).
    track(step('ChromaticAberration', fringeIntensity=5.0), 0.0, 0.3, parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    # The finisher: a zoom punch into the hit.
    track(step('CameraZoom', fieldOfViewChange=-14.0, easeInFraction=0.1, easeOutFraction=0.6), 0.0, 0.45, FLAT,
          parameterMappings=[mapping('Swing', FINISHER_ONLY)]),
    # Sounds: the blade, the body, a heavier body layer, the metal clash on the finisher.
    track(step('PlaySound', sound=snd('Sword_Hit'), placement='AttachedToTarget', pitchVariation=0.06, volumeMultiplier=0.8), 0.0, 0.4, FLAT),
    track(step('PlaySound', sound=snd('Body_Hit'), placement='AttachedToTarget', pitchVariation=0.08), 0.0, 0.3, FLAT),
    track(step('PlaySound', sound=snd('Body_Hit_Heavy'), placement='AttachedToTarget', pitchVariation=0.05), 0.0, 0.5, FLAT,
          parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    track(step('PlaySound', sound=snd('Sword_Clash'), placement='AttachedToTarget', pitchVariation=0.04, volumeMultiplier=0.8), 0.0, 0.6, FLAT,
          parameterMappings=[mapping('Swing', FINISHER_ONLY)]),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.8, rightSmall=0.8, shape='Kick'), 0.0, 0.28,
          parameterMappings=[mapping('Swing', [(0, 0.6), (1, 1)])]),
    # The charged strike: slow motion, a deep hit and a long rumble.
    track(step('SlowMoRamp', timeDilation=0.25, rampInTime=0.03, rampOutTime=0.3), 0.15, 0.6, FLAT,
          parameterMappings=[mapping('Charged', CHARGED_ONLY)]),
    track(step('PlaySound', sound=snd('Body_Hit_Heavy'), placement='AttachedToTarget', pitchMultiplier=0.7), 0.0, 0.7, FLAT,
          parameterMappings=[mapping('Charged', CHARGED_ONLY)]),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.5, rightSmall=0.5, shape='Smooth'), 0.1, 0.5,
          parameterMappings=[mapping('Charged', CHARGED_ONLY)]),
], parameters=[SWING, CHARGED])

recipe('EnemyHurt', 'Impact', 'What was hit reacts: a hard white flash, a freeze, a squash and wobble, sparks, and a streak counter from the second hit on.', [
    track(step('HitFlash', flashMaterial=HIT_FLASH, color=color(1, 1, 1), shape='Kick'), 0.0, 0.16, FLAT,
          parameterMappings=[mapping('Swing', [(0, 0.85), (1, 1)])]),
    track(step('ActorHitstop', timeDilation=0.01), 0.0, 0.12, FLAT),
    track(step('ActorHitstop', timeDilation=0.01), 0.12, 0.08, FLAT, parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    track(step('SquashStretch', axis='Z', amount=0.28, frequency=11.0, damping=6.0), 0.0, 0.45,
          parameterMappings=[mapping('Swing', [(0, 0.6), (1, 1.2)])]),
    track(step('MeshWobble', tiltAmplitude=rot(0, 0, 10), frequency=14.0, decay=2.5), 0.0, 0.4,
          parameterMappings=[mapping('Swing', [(0, 0.5), (1, 1)])]),
    sparks(1.6),
    sparks(2.6, parameterMappings=[mapping('Swing', HEAVY_ONLY)]),
    track(step('NumberPop', valueParameter='Streak', prefix='x', color=color(1, 0.85, 0.35), fontSize=34.0, popScale=1.9, riseDistance=80.0), 0.0, 0.8, FLAT,
          parameterMappings=[mapping('Streak', [(0, 0), (0.15, 0), (0.2, 1), (1, 1)])]),
], parameters=[SWING, STREAK])

recipe('EnemyDeath', 'Impact', 'An enemy goes down: a freeze, then a slow-motion beat with the color drained, a zoom, a heavy thud and a long rumble.', [
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.1, FLAT),
    track(step('SlowMoRamp', timeDilation=0.25, rampInTime=0.02, rampOutTime=0.35), 0.1, 0.8, FLAT),
    track(step('Desaturate', amount=0.9, shape='Smooth'), 0.0, 1.0, BELL),
    track(step('VignettePulse', vignetteIntensity=0.7, shape='Smooth'), 0.0, 1.0, BELL),
    track(step('CameraZoom', fieldOfViewChange=-10.0, easeInFraction=0.15, easeOutFraction=0.5), 0.0, 0.9, FLAT),
    track(step('CameraPunch', locationPunch=vec(0, 0, -10), rotationPunch=rot(-2.0, 0, 0), frequency=6.0, damping=7.0), 0.0, 0.45),
    track(step('PlaySound', sound=snd('Body_Fall'), placement='AttachedToTarget'), 0.05, 0.8, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Smooth'), 0.0, 0.45),
])


# Guards (D-069): a raised guard shows an energy shield; attacks meet it instead of the body. Camera moves are sized for
# the demo camera 3.5 m back: a block moves the view about 40% as far as a hit, a guard break about 70%.
POWER = parameter('Power', 0.5, 0.0, 1.0, 'How heavy the blocked swing was: 0.35 first swing, 0.65 second, 1 finisher. Passed by the guard.')
SHIELD_GLOW = [{'parameterName': 'Glow', 'bIsColor': False, 'scalarAmount': 4.0}]

recipe('GuardBlock', 'Impact', 'A guard holds. Plays on the shield: it flares, sparks fly, a clang that deepens with heavier swings, a short freeze, and the attacker\'s swing stops dead for a moment as if it bounced off.', [
    track(step('MaterialPulse', parameters=SHIELD_GLOW, shape='Kick', attackFraction=0.08), 0.0, 0.3, FLAT,
          parameterMappings=[mapping('Power', [(0, 0.5), (1, 1)])]),
    sparks(1.0),
    sparks(1.8, parameterMappings=[mapping('Power', HEAVY_ONLY)]),
    track(step('PlaySound', sound=snd('Guard_Clang_Light'), placement='AttachedToTarget', pitchVariation=0.06), 0.0, 0.3, FLAT,
          parameterMappings=[mapping('Power', [(0, 0.7), (1, 1)])]),
    track(step('PlaySound', sound=snd('Guard_Clang_Heavy'), placement='AttachedToTarget', pitchVariation=0.05), 0.0, 0.3, FLAT,
          parameterMappings=[mapping('Power', HEAVY_ONLY)]),
    track(step('PlaySound', sound=snd('Guard_Field'), placement='AttachedToTarget', volumeMultiplier=0.35, bStopAtTrackEnd=True, fadeOutTime=0.2), 0.0, 0.5, FLAT),
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.04, FLAT),
    track(step('GlobalHitstop', timeDilation=0.02), 0.04, 0.03, FLAT, parameterMappings=[mapping('Power', HEAVY_ONLY)]),
    track(step('GlobalHitstop', timeDilation=0.02), 0.07, 0.03, FLAT, parameterMappings=[mapping('Power', FINISHER_ONLY)]),
    track(step('ActorHitstop', timeDilation=0.02), 0.0, 0.12, FLAT, appliesTo='Instigator',
          parameterMappings=[mapping('Power', [(0, 0.8), (1, 1)])]),
    track(step('CameraPunch', locationPunch=vec(-16, 0, -8), rotationPunch=rot(-3.0, 0, 0), shape='Kick', attackFraction=0.15), 0.0, 0.35, FLAT,
          parameterMappings=[mapping('Power', [(0, 0.5), (1, 1.2)])]),
    track(step('ProceduralShake', frequency=16.0, rotationAmplitude=rot(0.7, 0.7, 0.35), locationAmplitude=vec(0, 8, 6)), 0.0, 0.2,
          parameterMappings=[mapping('Power', [(0, 0.5), (1, 1.2)])]),
    track(step('ForceFeedbackCurve', leftSmall=0.6, rightSmall=0.6, leftLarge=0.3, rightLarge=0.3, shape='Kick'), 0.0, 0.14,
          parameterMappings=[mapping('Power', [(0, 0.5), (1, 1)])]),
], parameters=[POWER])

recipe('GuardBreak', 'Power', 'A charged strike breaks a guard. Plays on the defender: the shield shatters over a heavy clang, a freeze into slow motion, a warm flash with color fringes, the defender flashes and buckles, a long rumble.', [
    track(step('PlaySound', sound=snd('Guard_Break'), placement='AttachedToTarget'), 0.0, 0.6, FLAT),
    track(step('PlaySound', sound=snd('Guard_Clang_Heavy'), placement='AttachedToTarget', pitchMultiplier=0.8), 0.0, 0.4, FLAT),
    track(step('PlaySound', sound=snd('Body_Hit_Heavy'), placement='AttachedToTarget', pitchMultiplier=0.85), 0.0, 0.5, FLAT),
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.1, FLAT),
    track(step('SlowMoRamp', timeDilation=0.3, rampInTime=0.02, rampOutTime=0.3), 0.1, 0.55, FLAT),
    track(step('ScreenFlash', color=color(1, 0.85, 0.6), maxOpacity=0.3), 0.0, 0.12),
    track(step('ChromaticAberration', fringeIntensity=5.0), 0.0, 0.35),
    track(step('CameraPunch', locationPunch=vec(-35, 0, -15), rotationPunch=rot(-6.0, 0, 0), shape='Kick', attackFraction=0.1), 0.0, 0.5, FLAT),
    track(step('ProceduralShake', frequency=14.0, rotationAmplitude=rot(1.4, 1.4, 0.7), locationAmplitude=vec(0, 12, 10)), 0.0, 0.3),
    sparks(2.6),
    sparks(1.6),
    track(step('HitFlash', flashMaterial=HIT_FLASH, color=color(1, 0.6, 0.3), shape='Kick'), 0.0, 0.22, FLAT),
    track(step('SquashStretch', axis='Z', amount=-0.12, shape='Kick', attackFraction=0.15), 0.0, 0.35, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.6, rightSmall=0.6, shape='Kick'), 0.0, 0.4),
])

recipe('Parry', 'Power', 'A guard raised just before the hit turns it aside. Plays on the one who parried: a clear ring, a longer freeze, a white flash and a small zoom; the attacker flashes white and freezes, staggered.', [
    track(step('PlaySound', sound=snd('Parry_Ring'), placement='AttachedToTarget', volumeMultiplier=0.9), 0.0, 1.0, FLAT),
    track(step('PlaySound', sound=snd('Guard_Clang_Light'), placement='AttachedToTarget', pitchMultiplier=1.25), 0.0, 0.3, FLAT),
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.14, FLAT),
    track(step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.3), 0.0, 0.06, FLAT),
    track(step('ChromaticAberration', fringeIntensity=3.0), 0.0, 0.25),
    track(step('CameraZoom', fieldOfViewChange=-8.0, easeInFraction=0.1, easeOutFraction=0.6), 0.0, 0.45, FLAT),
    sparks(2.2),
    track(step('HitFlash', flashMaterial=HIT_FLASH, color=color(1, 1, 1), shape='Kick'), 0.0, 0.2, FLAT, appliesTo='Instigator'),
    track(step('ActorHitstop', timeDilation=0.02), 0.0, 0.3, FLAT, appliesTo='Instigator'),
    track(step('ForceFeedbackCurve', leftSmall=0.9, rightSmall=0.9, leftLarge=0.4, rightLarge=0.4, shape='Kick'), 0.0, 0.12),
])


def write():
    if os.path.isdir(ROOT):
        shutil.rmtree(ROOT)
    os.makedirs(ROOT)
    for name, body in RECIPES:
        data = {'format': 'FeelKitRecipe', 'schemaVersion': 2, 'name': name, 'recipe': body}
        with open(os.path.join(ROOT, name + '.json'), 'w', encoding='utf-8') as file:
            json.dump(data, file, indent='\t')
            file.write('\n')
    print('wrote', len(RECIPES), 'recipes to', ROOT)


if __name__ == '__main__':
    write()
