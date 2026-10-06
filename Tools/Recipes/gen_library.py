"""Writes the FeelKit recipe library as JSON files (Plugins/FeelKit/Library/<Feeling>/FR_<Feeling>_<Name>.json).

The JSON files are the shipped source of the library; this script is only the authoring tool.
Run: python gen_library.py
"""
import json, os, shutil

ROOT = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Library'
S = '/Script/FeelCore.FeelStep_'
SOUND = '/FeelKit/Samples/Sounds/S_FK_{0}.S_FK_{0}'
DECAL_MAT = '/FeelKit/Samples/Materials/M_FK_Decal_Scorch.M_FK_Decal_Scorch'
PP_MAT = '/FeelKit/Samples/Materials/M_FK_PP_Pulse.M_FK_PP_Pulse'

CHANNEL = {
    'ProceduralShake': 'Feel.Camera.Shake',
    'CameraPunch': 'Feel.Camera.Motion', 'FOVKick': 'Feel.Camera.Motion', 'CameraRoll': 'Feel.Camera.Motion',
    'CameraZoom': 'Feel.Camera.Motion', 'LookAtNudge': 'Feel.Camera.Motion',
    'ScreenFlash': 'Feel.Screen.Flash',
    'VignettePulse': 'Feel.Screen.Distortion', 'ChromaticAberration': 'Feel.Screen.Distortion',
    'PostProcessMaterialPulse': 'Feel.Screen.Distortion',
    'Desaturate': 'Feel.Screen.Color', 'ColorTint': 'Feel.Screen.Color',
    'ScreenFade': 'Feel.Screen.Fade',
    'ScalePunch': 'Feel.Actor.Transform', 'SquashStretch': 'Feel.Actor.Transform', 'MeshWobble': 'Feel.Actor.Transform',
    'MaterialPulse': 'Feel.Actor.Material', 'HitFlash': 'Feel.Actor.Material',
    'LightFlash': 'Feel.Actor.Light',
    'GlobalHitstop': 'Feel.Time.Hitstop', 'ActorHitstop': 'Feel.Time.Hitstop',
    'SlowMoRamp': 'Feel.Time.SlowMo',
    'PlaySound': 'Feel.Audio',
    'SoundClassDuck': 'Feel.Audio.Mix', 'PitchBend': 'Feel.Audio.Mix', 'LowPassSweep': 'Feel.Audio.Mix',
    'ForceFeedbackCurve': 'Feel.Haptics', 'HapticPattern': 'Feel.Haptics',
    'WidgetPunch': 'Feel.UI', 'WidgetShake': 'Feel.UI', 'WidgetFlash': 'Feel.UI', 'NumberPop': 'Feel.UI',
    'SpawnDecal': 'Feel.Spawn',
    'Recipe': 'Feel.Meta.Recipe', 'RandomChoice': 'Feel.Meta.Recipe',
    'BlueprintEvent': 'Feel.Meta.Event',
}


def curve(points):
    """An intensity curve from (time, value) points, linear between them."""
    return {'editorCurveData': {'keys': [
        {'interpMode': 'RCIM_Linear', 'time': float(t), 'value': float(v)} for t, v in points]}}


FLAT = curve([(0, 1), (1, 1)])
FADE = curve([(0, 1), (1, 0)])
RISE = curve([(0, 0), (1, 1)])
SPIKE = curve([(0, 0.15), (0.12, 1), (1, 0)])
BELL = curve([(0, 0), (0.35, 1), (1, 0)])


def step(kind, **props):
    out = {'_ClassName': S + kind}
    out.update(props)
    out['__kind'] = kind
    return out


def track(step_dict, start, duration, intensity=None, **extra):
    kind = step_dict.pop('__kind')
    data = {
        'step': step_dict,
        'startTime': round(float(start), 4),
        'duration': round(float(duration), 4),
        'channel': {'tagName': CHANNEL[kind]},
        'intensityCurve': intensity if intensity is not None else FADE,
    }
    for value in extra.values():
        # Steps nested in a track (such as a substitute step) carry the helper key too; it is not a step property.
        if isinstance(value, dict):
            value.pop('__kind', None)
    data.update(extra)
    return data


def mapping(parameter, points=None):
    out = {'parameter': parameter}
    if points:
        out['curve'] = curve(points)
    return out


def parameter(name, default, low, high, description, accumulator=None):
    out = {'name': name, 'defaultValue': default, 'minValue': low, 'maxValue': high,
           'description': description}
    if accumulator:
        out['accumulator'] = accumulator
    return out


def color(r, g, b, a=1.0):
    return {'r': r, 'g': g, 'b': b, 'a': a}


def vec(x, y, z):
    return {'x': x, 'y': y, 'z': z}


def rot(pitch, yaw, roll):
    return {'pitch': pitch, 'yaw': yaw, 'roll': roll}


RECIPES = []


def recipe(feeling, name, description, genres, tracks, parameters=None, sustain=None, cooldown=0.0, default_intensity=1.0,
           release=None, jump_on_release=False, full_release=None, early_release=None):
    body = {
        'tracks': tracks,
        'cooldown': cooldown,
        'defaultIntensity': default_intensity,
        'feeling': {'tagName': 'Feel.Feeling.' + feeling},
        'genres': {'gameplayTags': [{'tagName': 'Feel.Genre.' + g} for g in genres]},
        'description': description,
    }
    if parameters:
        body['parameters'] = parameters
    if sustain:
        body['bSustain'] = True
        body['sustainStart'] = sustain[0]
        body['sustainEnd'] = sustain[1]
    if jump_on_release:
        body['bJumpToEndOnRelease'] = True
    if release:
        # (parameter, point in its range from 0 to 1) at which the play releases itself.
        body['releaseParameter'] = release[0]
        body['releaseAt'] = release[1]
    if full_release:
        body['fullReleaseRecipe'] = full_release
    if early_release:
        body['earlyReleaseRecipe'] = early_release
    RECIPES.append((feeling, 'FR_{0}_{1}'.format(feeling, name), body))


def uses_release_settings(body):
    """Whether a recipe needs schema 3: release by parameter, jump to end on release or a release recipe."""
    return any(key in body for key in ('releaseParameter', 'bJumpToEndOnRelease', 'fullReleaseRecipe', 'earlyReleaseRecipe'))


# --------------------------------------------------------------------------------------- Impact

recipe('Impact', 'LightHit', 'A quick, light hit: a short shake, a small punch and a brief freeze.', ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.15), 0.0, 0.04, FLAT),
    track(step('ProceduralShake', frequency=22.0, rotationAmplitude=rot(0.8, 0.8, 0.4), locationAmplitude=vec(0, 2, 2)), 0.0, 0.18),
    track(step('CameraPunch', locationPunch=vec(-6, 0, -2), rotationPunch=rot(-1.5, 0, 0), frequency=9.0, damping=9.0), 0.0, 0.25),
    track(step('ScalePunch', amount=vec(0.12, 0.12, 0.12), bounces=1), 0.0, 0.22),
    track(step('PlaySound', sound=SOUND.format('Hit_Light'), placement='AttachedToTarget'), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.35, rightLarge=0.35, leftSmall=0.5, rightSmall=0.5, shape='Kick'), 0.0, 0.12),
])

recipe('Impact', 'HeavyHit', 'A heavy blow: a longer freeze, a deep camera punch, a flash and a strong rumble.', ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.1, FLAT),
    track(step('ProceduralShake', frequency=16.0, rotationAmplitude=rot(2.2, 2.2, 1.2), locationAmplitude=vec(0, 6, 6)), 0.0, 0.35),
    track(step('CameraPunch', locationPunch=vec(-16, 0, -6), rotationPunch=rot(-4.0, 0, 0), frequency=6.0, damping=7.0), 0.0, 0.45),
    track(step('ScalePunch', amount=vec(0.3, 0.3, 0.3), bounces=1), 0.0, 0.4),
    track(step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.25), 0.0, 0.12),
    track(step('PlaySound', sound=SOUND.format('Hit_Heavy'), placement='AttachedToTarget'), 0.0, 0.5, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.6, rightSmall=0.6, shape='Kick'), 0.0, 0.3),
])

recipe('Impact', 'CriticalHit', 'A critical hit: a longer freeze, a slow-motion beat, a bright flash and a color push.', ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.02), 0.0, 0.14, FLAT),
    track(step('SlowMoRamp', timeDilation=0.35, rampInTime=0.05, rampOutTime=0.25), 0.14, 0.45, FLAT),
    track(step('ProceduralShake', frequency=18.0, rotationAmplitude=rot(3.0, 3.0, 1.5), locationAmplitude=vec(0, 8, 8)), 0.0, 0.45),
    track(step('CameraPunch', locationPunch=vec(-22, 0, -8), rotationPunch=rot(-5.0, 0, 0), frequency=5.5, damping=6.0), 0.0, 0.5),
    track(step('ScreenFlash', color=color(1, 0.95, 0.75), maxOpacity=0.4), 0.0, 0.18),
    track(step('ChromaticAberration', fringeIntensity=4.0), 0.0, 0.4),
    track(step('ScalePunch', amount=vec(0.4, 0.4, 0.4), bounces=2), 0.0, 0.5),
    track(step('PlaySound', sound=SOUND.format('Hit_Crit'), placement='AttachedToTarget'), 0.0, 0.6, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.8, rightSmall=0.8, shape='Kick', repeats=2), 0.0, 0.35),
])

recipe('Impact', 'ScalableHit', 'One hit that covers light to heavy: Damage drives how hard everything lands.',
       ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.08), 0.0, 0.08, FLAT, parameterMappings=[mapping('Damage', [(0, 0.35), (1, 1)])]),
    track(step('ProceduralShake', frequency=18.0, rotationAmplitude=rot(2.0, 2.0, 1.0), locationAmplitude=vec(0, 5, 5)), 0.0, 0.3,
          parameterMappings=[mapping('Damage', [(0, 0.3), (1, 1)])]),
    track(step('CameraPunch', locationPunch=vec(-14, 0, -5), rotationPunch=rot(-3.5, 0, 0), frequency=7.0, damping=7.5), 0.0, 0.4,
          parameterMappings=[mapping('Damage', [(0, 0.3), (1, 1)])]),
    track(step('ScalePunch', amount=vec(0.25, 0.25, 0.25), bounces=1), 0.0, 0.35,
          parameterMappings=[mapping('Damage', [(0, 0.4), (1, 1)])]),
    track(step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.3), 0.0, 0.12,
          parameterMappings=[mapping('Damage', [(0, 0), (0.6, 0.2), (1, 1)])]),
    track(step('PlaySound', sound=SOUND.format('Hit_Heavy'), placement='AttachedToTarget'), 0.0, 0.5, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.9, rightLarge=0.9, shape='Kick'), 0.0, 0.25,
          parameterMappings=[mapping('Damage', [(0, 0.3), (1, 1)])]),
], parameters=[parameter('Damage', 50.0, 0.0, 100.0, 'How much damage the hit did. 0 plays the lightest version, 100 the heaviest.')])

recipe('Impact', 'BulletImpact', 'A shot landing on a surface: a small punch, a mark on the surface and a short rumble.',
       ['Shooter'], [
    track(step('CameraPunch', locationPunch=vec(-4, 0, -1.5), rotationPunch=rot(-1.0, 0, 0), frequency=11.0, damping=10.0), 0.0, 0.18),
    track(step('SpawnDecal', decalMaterial=DECAL_MAT, decalSize=vec(12, 10, 10), lifetime=8.0, fadeOutTime=2.0), 0.0, 0.05, FLAT),
    track(step('PlaySound', sound=SOUND.format('Impact_Bullet'), placement='AtTargetLocation'), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.3, rightLarge=0.3, leftSmall=0.45, rightSmall=0.45, shape='Kick'), 0.0, 0.1),
])

# --------------------------------------------------------------------------------------- Weight

recipe('Weight', 'Land', 'Landing on the ground, scaled by how fast the fall was.', ['Platformer', 'Action'], [
    track(step('CameraPunch', locationPunch=vec(0, 0, -14), rotationPunch=rot(-2.5, 0, 0), frequency=8.0, damping=8.0), 0.0, 0.35,
          parameterMappings=[mapping('FallSpeed', [(0, 0.25), (1, 1)])]),
    track(step('SquashStretch', axis='Z', amount=0.25, frequency=9.0, damping=7.0), 0.0, 0.4,
          parameterMappings=[mapping('FallSpeed', [(0, 0.3), (1, 1)])]),
    track(step('ProceduralShake', frequency=14.0, rotationAmplitude=rot(1.2, 0.8, 0.6), locationAmplitude=vec(0, 2, 4)), 0.0, 0.25,
          parameterMappings=[mapping('FallSpeed', [(0, 0.2), (1, 1)])]),
    track(step('PlaySound', sound=SOUND.format('Land_Thud'), placement='AttachedToTarget'), 0.0, 0.4, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.7, rightLarge=0.7, shape='Kick'), 0.0, 0.2,
          parameterMappings=[mapping('FallSpeed', [(0, 0.2), (1, 1)])]),
], parameters=[parameter('FallSpeed', 600.0, 0.0, 1600.0, 'Downward speed when landing, in centimeters per second.')])

recipe('Weight', 'Stomp', 'A deliberate, heavy stomp: a freeze, a drop of the camera and a long rumble.', ['Action'], [
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.08, FLAT),
    track(step('CameraPunch', locationPunch=vec(0, 0, -22), rotationPunch=rot(-3.5, 0, 0), frequency=6.0, damping=6.5), 0.0, 0.5),
    track(step('SquashStretch', axis='Z', amount=0.35, frequency=7.0, damping=6.0), 0.0, 0.5),
    track(step('ProceduralShake', frequency=11.0, rotationAmplitude=rot(1.8, 1.2, 1.0), locationAmplitude=vec(0, 3, 6)), 0.0, 0.5),
    track(step('PlaySound', sound=SOUND.format('Land_Thud'), placement='AttachedToTarget', pitchMultiplier=0.85), 0.0, 0.5, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.3, rightSmall=0.3, shape='Smooth'), 0.0, 0.4),
])

recipe('Weight', 'HeavyFootstep', 'Each step of something big: a small ground shake felt more than seen.', ['Action', 'Horror'], [
    track(step('ProceduralShake', frequency=9.0, rotationAmplitude=rot(0.5, 0.3, 0.3), locationAmplitude=vec(0, 1, 3)), 0.0, 0.3),
    track(step('CameraPunch', locationPunch=vec(0, 0, -5), rotationPunch=rot(-0.8, 0, 0), frequency=7.0, damping=9.0), 0.0, 0.25),
    track(step('ForceFeedbackCurve', leftLarge=0.5, rightLarge=0.5, leftSmall=0.0, rightSmall=0.0, shape='Smooth'), 0.0, 0.25),
])

recipe('Weight', 'Slam', 'Something heavy hitting the ground nearby: a hard shake and a mark left behind.', ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.1), 0.0, 0.06, FLAT),
    track(step('ProceduralShake', frequency=13.0, rotationAmplitude=rot(2.5, 1.5, 1.2), locationAmplitude=vec(0, 5, 9)), 0.0, 0.6),
    track(step('CameraPunch', locationPunch=vec(0, 0, -18), rotationPunch=rot(-3.0, 0, 0), frequency=5.5, damping=6.0), 0.0, 0.55),
    track(step('SpawnDecal', decalMaterial=DECAL_MAT, decalSize=vec(20, 90, 90), lifetime=10.0, fadeOutTime=3.0), 0.0, 0.05, FLAT),
    track(step('PlaySound', sound=SOUND.format('Land_Thud'), placement='AtTargetLocation', pitchMultiplier=0.8), 0.0, 0.6, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Kick'), 0.0, 0.35),
])

# --------------------------------------------------------------------------------------- Power

recipe('Power', 'ChargeUp', 'Holding a charge: a rising hum, a tightening view and a growing rumble. At full charge it releases itself and plays FR_Power_ChargedRelease; let go early and the hum fades out.',
       ['Action', 'Shooter'], [
    track(step('FOVKick', fieldOfViewKick=-6.0, shape='Smooth', frequency=2.0, damping=3.0), 0.0, 1.0, RISE,
          parameterMappings=[mapping('Charge', [(0, 0.35), (1, 1)])]),
    track(step('VignettePulse', vignetteIntensity=0.8, shape='Smooth'), 0.0, 1.0, RISE,
          parameterMappings=[mapping('Charge', [(0, 0.3), (1, 1)])]),
    track(step('ProceduralShake', frequency=20.0, rotationAmplitude=rot(0.4, 0.4, 0.2), locationAmplitude=vec(0, 1, 1)), 0.0, 1.0, RISE,
          parameterMappings=[mapping('Charge', [(0, 0.2), (1, 1)])]),
    track(step('PlaySound', sound=SOUND.format('Charge_Loop'), placement='AttachedToTarget', bStopAtTrackEnd=True, fadeOutTime=0.2), 0.0, 1.0, FLAT,
          parameterMappings=[mapping('Charge', [(0, 0.6), (1, 1)])]),
    track(step('ForceFeedbackCurve', leftLarge=0.0, rightLarge=0.0, leftSmall=0.8, rightSmall=0.8, shape='Smooth'), 0.0, 1.0, RISE,
          parameterMappings=[mapping('Charge', [(0, 0.3), (1, 1)])]),
], sustain=(0.35, 0.95), parameters=[parameter('Charge', 0.0, 0.0, 1.0, 'How far the charge has come, from 0 to 1. At 1 the play releases itself.')],
   # Charge reaching 1 releases the play and plays the burst (On Full Release); a release from the game before that ends
   # the loop at once and lets the hum fade.
   release=('Charge', 1.0), full_release='/FeelKit/Library/Power/FR_Power_ChargedRelease.FR_Power_ChargedRelease')

recipe('Power', 'ChargedRelease', 'The charge let go: a flash, a wide camera kick and a heavy rumble.', ['Action', 'Shooter'], [
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.07, FLAT),
    track(step('ScreenFlash', color=color(1, 0.92, 0.7), maxOpacity=0.45), 0.0, 0.2),
    track(step('FOVKick', fieldOfViewKick=12.0, frequency=5.0, damping=6.0), 0.0, 0.5),
    track(step('CameraPunch', locationPunch=vec(-20, 0, 0), rotationPunch=rot(-3.0, 0, 0), frequency=6.0, damping=7.0), 0.0, 0.45),
    track(step('ProceduralShake', frequency=15.0, rotationAmplitude=rot(2.0, 2.0, 1.0), locationAmplitude=vec(0, 6, 6)), 0.0, 0.5),
    track(step('ChromaticAberration', fringeIntensity=5.0), 0.0, 0.4),
    track(step('PlaySound', sound=SOUND.format('Explosion'), placement='AttachedToTarget'), 0.0, 0.6, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.7, rightSmall=0.7, shape='Kick'), 0.0, 0.4),
])

recipe('Power', 'Explosion', 'A blast going off nearby: the closer it is, the harder it hits.', ['Shooter', 'Action'], [
    track(step('ScreenFlash', color=color(1, 0.85, 0.6), maxOpacity=0.5), 0.0, 0.15,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
    track(step('CameraPunch', locationPunch=vec(-25, 0, -6), rotationPunch=rot(-4.0, 0, 0), frequency=5.0, damping=6.0,
               directionSource='AwayFromPlayLocation'), 0.0, 0.6,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
    track(step('ProceduralShake', frequency=14.0, rotationAmplitude=rot(3.0, 3.0, 2.0), locationAmplitude=vec(0, 8, 8)), 0.0, 0.7,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
    track(step('ChromaticAberration', fringeIntensity=6.0), 0.0, 0.5,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
    track(step('LowPassSweep', cutoffFrequency=600.0, attackFraction=0.05, releaseFraction=0.6), 0.0, 1.2,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
    track(step('PlaySound', sound=SOUND.format('Explosion'), placement='AtTargetLocation'), 0.0, 0.8, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Kick'), 0.0, 0.5,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0)])]),
], parameters=[parameter('Distance', 0.0, 0.0, 2000.0, 'How far the player is from the blast, in centimeters. FeelKit fills this in when the game does not.')])

recipe('Power', 'AbilityCast', 'Casting an ability: a short wind-up, a colored wash and a firm rumble.', ['Action'], [
    track(step('ColorTint', tintColor=color(0.4, 0.6, 1.0), strength=0.5, shape='Smooth'), 0.0, 0.5, BELL),
    track(step('FOVKick', fieldOfViewKick=-4.0, shape='Smooth', frequency=3.0, damping=5.0), 0.0, 0.4),
    track(step('CameraPunch', locationPunch=vec(-6, 0, 2), rotationPunch=rot(1.5, 0, 0), frequency=7.0, damping=8.0), 0.25, 0.35),
    track(step('ScalePunch', amount=vec(0.15, 0.15, 0.2), bounces=1), 0.25, 0.35),
    track(step('PlaySound', sound=SOUND.format('LevelUp'), placement='AttachedToTarget', pitchMultiplier=0.9), 0.0, 0.6, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.5, rightLarge=0.5, leftSmall=0.6, rightSmall=0.6, shape='Smooth'), 0.0, 0.5, BELL),
])

# --------------------------------------------------------------------------------------- Speed

recipe('Speed', 'Dash', 'A burst of speed in the direction of travel: a camera push, a widening view and a whoosh.',
       ['Platformer', 'Action'], [
    track(step('CameraPunch', locationPunch=vec(18, 0, 0), rotationPunch=rot(1.0, 0, 0), frequency=7.0, damping=8.0,
               directionSource='PlayDirection'), 0.0, 0.35),
    track(step('FOVKick', fieldOfViewKick=10.0, frequency=6.0, damping=7.0), 0.0, 0.4),
    track(step('ChromaticAberration', fringeIntensity=3.0), 0.0, 0.35),
    track(step('SquashStretch', axis='X', amount=0.2, frequency=8.0, damping=7.0), 0.0, 0.3),
    track(step('PlaySound', sound=SOUND.format('Whoosh'), placement='AttachedToTarget'), 0.0, 0.4, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.4, rightLarge=0.4, leftSmall=0.5, rightSmall=0.5, shape='Kick'), 0.0, 0.2),
])

recipe('Speed', 'Boost', 'Sustained speed: the view widens and the edges blur while the boost lasts.', ['Vehicle', 'Platformer'], [
    track(step('FOVKick', fieldOfViewKick=14.0, shape='Smooth', frequency=2.5, damping=4.0), 0.0, 1.0, RISE),
    track(step('ChromaticAberration', fringeIntensity=4.0, shape='Smooth'), 0.0, 1.0, RISE),
    track(step('VignettePulse', vignetteIntensity=0.5, shape='Smooth'), 0.0, 1.0, RISE),
    track(step('ProceduralShake', frequency=24.0, rotationAmplitude=rot(0.3, 0.3, 0.2), locationAmplitude=vec(0, 1, 1)), 0.0, 1.0, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.0, rightLarge=0.0, leftSmall=0.5, rightSmall=0.5, shape='Smooth'), 0.0, 1.0, FLAT),
], sustain=(0.3, 0.9))

recipe('Speed', 'Whoosh', 'Something fast passing close by: a light camera turn and a sweep of sound.', ['Action', 'Shooter'], [
    track(step('CameraRoll', rollDegrees=2.5, frequency=6.0, damping=8.0), 0.0, 0.3),
    track(step('LookAtNudge', turnFraction=0.15, maxTurnDegrees=4.0, frequency=6.0, damping=8.0), 0.0, 0.3),
    track(step('PlaySound', sound=SOUND.format('Whoosh'), placement='AtTargetLocation'), 0.0, 0.35, FLAT),
])

recipe('Speed', 'SprintStart', 'Breaking into a sprint: a short lean forward, a widening view and a brief shake.', ['Action', 'Shooter'], [
    track(step('FOVKick', fieldOfViewKick=6.0, shape='Smooth', frequency=4.0, damping=6.0), 0.0, 0.5),
    track(step('CameraPunch', locationPunch=vec(6, 0, -2), rotationPunch=rot(1.0, 0, 0), frequency=6.0, damping=8.0), 0.0, 0.35),
    track(step('ProceduralShake', frequency=18.0, rotationAmplitude=rot(0.4, 0.4, 0.2), locationAmplitude=vec(0, 1, 2)), 0.0, 0.5),
])

# --------------------------------------------------------------------------------------- Reward

recipe('Reward', 'Pickup', 'Collecting something: a bright, short pop with a rising sound.', ['Platformer', 'Action', 'Shooter'], [
    track(step('ScalePunch', amount=vec(0.2, 0.2, 0.2), bounces=1), 0.0, 0.25),
    track(step('ScreenFlash', color=color(1, 0.95, 0.7), maxOpacity=0.12), 0.0, 0.1),
    track(step('CameraPunch', locationPunch=vec(0, 0, 3), rotationPunch=rot(0.8, 0, 0), frequency=10.0, damping=9.0), 0.0, 0.2),
    track(step('PlaySound', sound=SOUND.format('Pickup'), placement='AtTargetLocation'), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.0, rightLarge=0.0, leftSmall=0.35, rightSmall=0.35, shape='Kick'), 0.0, 0.1),
])

recipe('Reward', 'ComboStep', 'Each step of a streak hits harder than the last: the pop, the camera bump and the rumble grow with the combo, and the count pops up on screen.',
       ['Platformer', 'Action', 'UI'], [
    track(step('ScalePunch', amount=vec(0.18, 0.18, 0.18), bounces=1), 0.0, 0.25,
          parameterMappings=[mapping('Combo', [(0, 0.6), (1, 1.4)])]),
    track(step('CameraPunch', locationPunch=vec(0, 0, 4), rotationPunch=rot(1.0, 0, 0), frequency=10.0, damping=9.0), 0.0, 0.25,
          parameterMappings=[mapping('Combo', [(0, 0.4), (1, 1.2)])]),
    track(step('NumberPop', valueParameter='Combo', suffix='x', color=color(1, 0.9, 0.4), fontSize=32.0, popScale=1.8), 0.0, 0.8, FLAT),
    track(step('PlaySound', sound=SOUND.format('Pickup'), placement='AtTargetLocation', pitchMultiplier=1.0, pitchVariation=0.0), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftSmall=0.4, rightSmall=0.4, leftLarge=0.0, rightLarge=0.0, shape='Kick'), 0.0, 0.12,
          parameterMappings=[mapping('Combo', [(0, 0.4), (1, 1)])]),
], parameters=[parameter('Combo', 1.0, 0.0, 10.0, 'How long the streak is. Reads the Combo accumulator when the game does not pass it.', accumulator='Combo')])

recipe('Reward', 'LevelUp', 'A milestone reached: a warm flash, a lift of the camera and a triumphant sound.', ['Action', 'UI'], [
    track(step('ScreenFlash', color=color(1, 0.9, 0.55), maxOpacity=0.3), 0.0, 0.35),
    track(step('ColorTint', tintColor=color(1, 0.85, 0.5), strength=0.4, shape='Smooth'), 0.0, 0.9, BELL),
    track(step('CameraPunch', locationPunch=vec(0, 0, 10), rotationPunch=rot(2.0, 0, 0), frequency=4.0, damping=6.0), 0.0, 0.7),
    track(step('ScalePunch', amount=vec(0.25, 0.25, 0.3), bounces=2), 0.0, 0.8),
    track(step('PlaySound', sound=SOUND.format('LevelUp'), placement='TwoD'), 0.0, 1.0, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.6, rightLarge=0.6, leftSmall=0.5, rightSmall=0.5, shape='Smooth', repeats=2), 0.0, 0.6),
])

recipe('Reward', 'KillConfirm', 'Confirmation that something went down: a crisp double tick and a short freeze.',
       ['Shooter', 'Action'], [
    track(step('GlobalHitstop', timeDilation=0.1), 0.0, 0.05, FLAT),
    track(step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.15), 0.0, 0.08),
    track(step('CameraPunch', locationPunch=vec(-5, 0, 2), rotationPunch=rot(-1.0, 0, 0), frequency=12.0, damping=10.0), 0.0, 0.2),
    track(step('PlaySound', sound=SOUND.format('Hit_Crit'), placement='TwoD', pitchMultiplier=1.15), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftSmall=0.6, rightSmall=0.6, leftLarge=0.2, rightLarge=0.2, shape='Kick', repeats=2), 0.0, 0.2),
])

# --------------------------------------------------------------------------------------- Danger

recipe('Danger', 'DamageTaken', 'Being hurt: a red wash at the edges, a jolt and a low rumble.', ['Action', 'Shooter', 'Horror'], [
    track(step('ScreenFlash', color=color(0.8, 0.05, 0.05), maxOpacity=0.35), 0.0, 0.25, bEssential=True,
          substituteStep=step('VignettePulse', vignetteIntensity=1.2), essentialFloor=0.35),
    track(step('VignettePulse', vignetteIntensity=1.0), 0.0, 0.5),
    track(step('CameraPunch', locationPunch=vec(-8, 4, -3), rotationPunch=rot(-2.0, 1.5, 0), frequency=7.0, damping=7.0), 0.0, 0.4),
    track(step('ProceduralShake', frequency=15.0, rotationAmplitude=rot(1.5, 1.5, 0.8), locationAmplitude=vec(0, 4, 4)), 0.0, 0.3),
    track(step('ForceFeedbackCurve', leftLarge=0.8, rightLarge=0.8, shape='Kick'), 0.0, 0.3),
])

recipe('Danger', 'DirectionalDamage', 'Being hurt from a direction: the camera is pushed away from where it came from.',
       ['Shooter', 'Action'], [
    track(step('CameraPunch', locationPunch=vec(-12, 0, -3), rotationPunch=rot(-2.5, 0, 0), frequency=6.5, damping=7.0,
               directionSource='AwayFromPlayLocation'), 0.0, 0.45),
    track(step('CameraRoll', rollDegrees=3.0, bRandomDirection=False, frequency=5.0, damping=7.0), 0.0, 0.45),
    track(step('ScreenFlash', color=color(0.8, 0.05, 0.05), maxOpacity=0.3), 0.0, 0.22, bEssential=True,
          substituteStep=step('VignettePulse', vignetteIntensity=1.2), essentialFloor=0.35),
    track(step('ForceFeedbackCurve', leftLarge=0.9, rightLarge=0.5, shape='Kick'), 0.0, 0.3),
])

recipe('Danger', 'LowHealth', 'Running low: a slow pulse at the edges and a heartbeat that rises as health falls.',
       ['Action', 'Shooter', 'Horror'], [
    track(step('VignettePulse', vignetteIntensity=1.2, shape='Smooth', frequency=1.6, damping=0.6, repeats=4), 0.0, 1.2, FLAT,
          parameterMappings=[mapping('Health', [(0, 1), (1, 0)])]),
    track(step('Desaturate', amount=0.5, shape='Smooth'), 0.0, 1.2, FLAT,
          parameterMappings=[mapping('Health', [(0, 1), (1, 0)])]),
    track(step('PlaySound', sound=SOUND.format('Heartbeat'), placement='TwoD', bStopAtTrackEnd=True, fadeOutTime=0.4), 0.0, 1.2, FLAT,
          parameterMappings=[mapping('Health', [(0, 1), (1, 0)])]),
    track(step('ForceFeedbackCurve', leftLarge=0.5, rightLarge=0.0, leftSmall=0.0, rightSmall=0.0, shape='Smooth', repeats=2), 0.0, 1.2, FLAT,
          parameterMappings=[mapping('Health', [(0, 1), (1, 0)])]),
], sustain=(0.1, 1.1), parameters=[parameter('Health', 0.3, 0.0, 1.0, 'Health left, from 0 (empty) to 1 (full). Lower health makes the pulse stronger.')])

recipe('Danger', 'Alarm', 'An alarm going off: a repeating red wash and a warning tone.', ['Horror', 'Shooter'], [
    track(step('ColorTint', tintColor=color(1, 0.15, 0.1), strength=0.55, shape='Smooth', frequency=1.2, damping=0.5, repeats=3), 0.0, 1.5, FLAT),
    track(step('VignettePulse', vignetteIntensity=0.9, shape='Smooth', frequency=1.2, damping=0.5, repeats=3), 0.0, 1.5, FLAT),
    track(step('PlaySound', sound=SOUND.format('Alarm'), placement='TwoD', bStopAtTrackEnd=True, fadeOutTime=0.3), 0.0, 1.5, FLAT),
], sustain=(0.05, 1.45))

# --------------------------------------------------------------------------------------- Dread

recipe('Dread', 'Heartbeat', 'A heartbeat that follows fear: the beat, the pulse at the edges, the rumble and a slight zoom grow stronger the closer the threat.', ['Horror'], [
    track(step('PlaySound', sound=SOUND.format('Heartbeat'), placement='TwoD', bStopAtTrackEnd=True, fadeOutTime=0.5), 0.0, 1.5, FLAT,
          parameterMappings=[mapping('Fear')]),
    track(step('VignettePulse', vignetteIntensity=1.0, shape='Smooth', frequency=1.8, damping=0.4, repeats=3), 0.0, 1.5, FLAT,
          parameterMappings=[mapping('Fear')]),
    track(step('ForceFeedbackCurve', leftLarge=0.6, rightLarge=0.0, leftSmall=0.0, rightSmall=0.0, shape='Smooth', repeats=3), 0.0, 1.5, FLAT,
          parameterMappings=[mapping('Fear')]),
    track(step('CameraZoom', fieldOfViewChange=-3.0, easeInFraction=0.3, easeOutFraction=0.4), 0.0, 1.5, FLAT,
          parameterMappings=[mapping('Fear', [(0, 0), (0.6, 0.3), (1, 1)])]),
], sustain=(0.1, 1.4), parameters=[parameter('Fear', 0.5, 0.0, 1.0, 'How near the threat feels, from 0 (calm) to 1 (terrified).')])

recipe('Dread', 'Unease', 'Creeping unease: color drains, the edges close in and the world sounds muffled.', ['Horror'], [
    track(step('Desaturate', amount=0.7, shape='Smooth'), 0.0, 2.0, RISE),
    track(step('VignettePulse', vignetteIntensity=1.1, shape='Smooth'), 0.0, 2.0, RISE),
    track(step('LowPassSweep', cutoffFrequency=900.0, attackFraction=0.3, releaseFraction=0.5), 0.0, 2.0, RISE),
    track(step('CameraZoom', fieldOfViewChange=-5.0, easeInFraction=0.4, easeOutFraction=0.4), 0.0, 2.0, FLAT),
], sustain=(0.6, 1.9))

recipe('Dread', 'JumpScare', 'A sudden scare: one hard flash and a stab of sound, with a safe substitute when flashes are turned down.',
       ['Horror'], [
    track(step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.7), 0.0, 0.18, bEssential=True,
          substituteStep=step('VignettePulse', vignetteIntensity=1.5), essentialFloor=0.3),
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.08, FLAT),
    track(step('CameraPunch', locationPunch=vec(-14, 0, 6), rotationPunch=rot(3.0, 0, 0), frequency=8.0, damping=6.0), 0.0, 0.5),
    track(step('ProceduralShake', frequency=20.0, rotationAmplitude=rot(2.5, 2.5, 1.5), locationAmplitude=vec(0, 6, 6)), 0.0, 0.45),
    track(step('PlaySound', sound=SOUND.format('Alarm'), placement='TwoD', pitchMultiplier=1.3), 0.0, 0.6, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Kick'), 0.0, 0.35),
])

recipe('Dread', 'FailingLight', 'A light about to die: irregular flicker with a dip in color.', ['Horror'], [
    track(step('LightFlash', intensityChange=-0.8, bFlicker=True, flickerRate=14.0, shape='Smooth'), 0.0, 1.6, FLAT),
    track(step('Desaturate', amount=0.4, shape='Smooth'), 0.0, 1.6, FLAT),
], sustain=(0.2, 1.5))

# --------------------------------------------------------------------------------------- Denial

recipe('Denial', 'Blocked', 'An action that will not happen: a short stop and a dull thud.', ['Action', 'UI'], [
    track(step('CameraPunch', locationPunch=vec(-4, 0, 0), rotationPunch=rot(0.5, 0, 0), frequency=14.0, damping=12.0), 0.0, 0.18),
    track(step('ScalePunch', amount=vec(-0.08, -0.08, -0.08), bounces=0), 0.0, 0.2),
    track(step('PlaySound', sound=SOUND.format('Denied'), placement='TwoD'), 0.0, 0.25, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.3, rightLarge=0.3, leftSmall=0.0, rightSmall=0.0, shape='Kick'), 0.0, 0.12),
])

recipe('Denial', 'OutOfAmmo', 'Nothing left to fire: a dry click and a small shake of refusal.', ['Shooter'], [
    track(step('WidgetShake', amplitude={'x': 5.0, 'y': 0.0}, frequency=26.0, decay=2.0), 0.0, 0.25, FLAT),
    track(step('PlaySound', sound=SOUND.format('Denied'), placement='TwoD', pitchMultiplier=1.2), 0.0, 0.2, FLAT),
    track(step('ForceFeedbackCurve', leftSmall=0.3, rightSmall=0.3, leftLarge=0.0, rightLarge=0.0, shape='Kick'), 0.0, 0.08),
])

recipe('Denial', 'Locked', 'Something that will not open: a heavy rattle that goes nowhere.', ['Action', 'Horror'], [
    track(step('CameraPunch', locationPunch=vec(-3, 0, -1), rotationPunch=rot(-0.6, 0, 0), frequency=16.0, damping=10.0), 0.0, 0.3),
    track(step('ProceduralShake', frequency=22.0, rotationAmplitude=rot(0.4, 0.4, 0.2), locationAmplitude=vec(0, 1, 1)), 0.0, 0.3),
    track(step('PlaySound', sound=SOUND.format('Denied'), placement='AtTargetLocation', pitchMultiplier=0.85), 0.0, 0.35, FLAT),
])

recipe('Denial', 'WrongInput', 'The wrong button: a red shake on the element that refused it.', ['UI'], [
    track(step('WidgetShake', amplitude={'x': 9.0, 'y': 0.0}, frequency=30.0, decay=1.5), 0.0, 0.3, FLAT),
    track(step('WidgetFlash', color=color(1, 0.2, 0.2), strength=0.8, shape='Kick'), 0.0, 0.3),
    track(step('PlaySound', sound=SOUND.format('Denied'), placement='TwoD'), 0.0, 0.25, FLAT),
])

# --------------------------------------------------------------------------------------- Interface

recipe('Interface', 'ButtonHover', 'The cursor arrives on a button: a small lift and a soft tick.', ['UI'], [
    track(step('WidgetPunch', scaleChange={'x': 0.06, 'y': 0.06}, frequency=9.0, damping=9.0), 0.0, 0.2),
    track(step('PlaySound', sound=SOUND.format('UI_Hover'), placement='TwoD'), 0.0, 0.2, FLAT),
])

recipe('Interface', 'ButtonPress', 'A button taking the press: a squash inward, then a bounce back.', ['UI'], [
    track(step('WidgetPunch', scaleChange={'x': -0.12, 'y': -0.12}, frequency=12.0, damping=10.0), 0.0, 0.22),
    track(step('WidgetFlash', color=color(1, 1, 1), strength=0.4, shape='Kick'), 0.0, 0.15),
    track(step('PlaySound', sound=SOUND.format('UI_Click'), placement='TwoD'), 0.0, 0.2, FLAT),
    track(step('ForceFeedbackCurve', leftSmall=0.25, rightSmall=0.25, leftLarge=0.0, rightLarge=0.0, shape='Kick'), 0.0, 0.08),
])

recipe('Interface', 'Notification', 'Something arrives on screen: it slides in, settles and chimes.', ['UI'], [
    track(step('WidgetPunch', scaleChange={'x': 0.1, 'y': 0.1}, translation={'x': 0.0, 'y': -24.0}, frequency=6.0, damping=7.0), 0.0, 0.45),
    track(step('PlaySound', sound=SOUND.format('Pickup'), placement='TwoD', pitchMultiplier=1.1), 0.0, 0.35, FLAT),
])

recipe('Interface', 'ScoreTick', 'A score counting up: the points pop up, the counter punches and a short click plays.', ['UI'], [
    track(step('NumberPop', valueParameter='Combo', prefix='+', color=color(1, 0.92, 0.5), fontSize=26.0, popScale=1.5, riseDistance=40.0), 0.0, 0.6, FLAT),
    track(step('WidgetPunch', scaleChange={'x': 0.08, 'y': 0.08}, frequency=13.0, damping=10.0), 0.0, 0.2),
    track(step('PlaySound', sound=SOUND.format('UI_Click'), placement='TwoD', pitchMultiplier=1.2, pitchVariation=0.0), 0.0, 0.15, FLAT),
], parameters=[parameter('Combo', 1.0, 0.0, 10.0, 'The number shown and how big the pop is. Reads the Combo accumulator when the game does not pass it.', accumulator='Combo')])

recipe('Interface', 'ScreenTransition', 'Moving between screens: a quick fade out and back in.', ['UI'], [
    track(step('ScreenFade', fadeColor=color(0, 0, 0), maxOpacity=1.0, fadeInFraction=0.35, fadeOutFraction=0.35), 0.0, 0.7, FLAT),
    track(step('PlaySound', sound=SOUND.format('Whoosh'), placement='TwoD', pitchMultiplier=1.1), 0.0, 0.4, FLAT),
])


def write():
    if os.path.isdir(ROOT):
        shutil.rmtree(ROOT)
    for feeling, name, body in RECIPES:
        folder = os.path.join(ROOT, feeling)
        os.makedirs(folder, exist_ok=True)
        data = {'format': 'FeelKitRecipe', 'schemaVersion': 3 if uses_release_settings(body) else 2, 'name': name, 'recipe': body}
        with open(os.path.join(folder, name + '.json'), 'w', encoding='utf-8') as file:
            json.dump(data, file, indent='\t')
            file.write('\n')
    print('wrote', len(RECIPES), 'recipes')
    counts = {}
    for feeling, _, _ in RECIPES:
        counts[feeling] = counts.get(feeling, 0) + 1
    print(counts)


if __name__ == '__main__':
    write()
