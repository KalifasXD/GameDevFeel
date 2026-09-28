"""Writes the Horror demo kit recipes as JSON (Plugins/FeelKit/Demos/Horror/FR_HOR_<Name>.json).

Uses the helpers of gen_library.py. Run: python gen_demo_horror.py
Built from the library's Dread recipes. Kept low-key on purpose: the quiet between moments is part of it.
Every moment has a visible source (a lamp, a doorway, your own breath), so nothing sounds out of nowhere.
"""
import json, os, shutil
from gen_library import step, track, mapping, parameter, color, vec, rot, curve, FLAT, FADE, BELL, RISE, SOUND

ROOT = os.path.join('B:', os.sep, 'NewUE5Project', 'GameFeelDev', 'Plugins', 'FeelKit', 'Demos', 'Horror')
ATT = '/FeelKit/Samples/Sounds/ATT_FK_World.ATT_FK_World'

LAND_SPEED = parameter('LandSpeed', 500.0, 300.0, 1400.0, 'Downward speed at landing in cm/s, passed by the Feel Trigger Landed event.')

RECIPES = []


def recipe(name, feeling, description, tracks, sustain=None, cooldown=0.0, parameters=None):
    body = {
        'tracks': tracks,
        'cooldown': cooldown,
        'defaultIntensity': 1.0,
        'feeling': {'tagName': 'Feel.Feeling.' + feeling},
        'genres': {'gameplayTags': [{'tagName': 'Feel.Genre.Horror'}]},
        'description': description,
    }
    if sustain:
        body['bSustain'] = True
        body['sustainStart'], body['sustainEnd'] = sustain
    if parameters:
        body['parameters'] = parameters
    RECIPES.append(('FR_HOR_' + name, body))


# Sustained between 0.4 and 1.4 s while sprinting, so the footfalls keep repeating with the run.
recipe('Sprint', 'Speed', 'While sprinting: the view widens a touch, sways with the run, and heavy footfalls carry through the floor.', [
    track(step('CameraZoom', fieldOfViewChange=4.0, easeInFraction=0.2, easeOutFraction=0.2), 0.0, 1.8, FLAT),
    track(step('ProceduralShake', frequency=2.2, rotationAmplitude=rot(0.35, 0.25, 0.3), locationAmplitude=vec(0, 0.6, 0.8)), 0.0, 1.8, FLAT),
    track(step('PlaySound', sound=SOUND.format('Land_Step'), placement='TwoD', volumeMultiplier=0.3, pitchMultiplier=0.85, pitchVariation=0.12), 0.45, 0.15, FLAT),
    track(step('PlaySound', sound=SOUND.format('Land_Step'), placement='TwoD', volumeMultiplier=0.3, pitchMultiplier=0.8, pitchVariation=0.12), 0.95, 0.15, FLAT),
], sustain=(0.4, 1.4))

# One heartbeat cycle per sustain loop (0.1 to 1.0 s); after release the last beats fade over a second.
recipe('OutOfBreath', 'Dread', 'Out of breath: a heavy heartbeat, the edges close in, color drains a little, the world sounds muffled, and the view heaves with each breath until the stamina has recovered.', [
    track(step('PlaySound', sound=SOUND.format('Heartbeat'), placement='TwoD', volumeMultiplier=0.8, bStopAtTrackEnd=True, fadeOutTime=0.6), 0.0, 2.0, FLAT),
    track(step('VignettePulse', vignetteIntensity=0.7, shape='Smooth', frequency=1.1, damping=0.4, repeats=2), 0.0, 2.0, FLAT),
    track(step('Desaturate', amount=0.35, shape='Smooth'), 0.0, 2.0, FLAT),
    track(step('LowPassSweep', cutoffFrequency=1200.0, attackFraction=0.2, releaseFraction=0.3), 0.0, 2.0, FLAT),
    track(step('ProceduralShake', frequency=0.7, rotationAmplitude=rot(0.8, 0.2, 0.2), locationAmplitude=vec(0, 0, 1.5)), 0.0, 2.0, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.25, shape='Smooth', repeats=2), 0.0, 1.0),
], sustain=(0.1, 1.0))

# HUD (D-064): the sprint meter beats with the heartbeat while it refills from empty (same loop as OutOfBreath, two
# beats per loop), then brightens once when it is full again. Played by the Horror UI on the meter's frame.
def vec2(x, y):
    return {'x': float(x), 'y': float(y)}


recipe('HudBreathless', 'Dread', 'The sprint meter ran empty: it beats red with the heartbeat until it has refilled.', [
    track(step('WidgetFlash', color=color(1, 0.12, 0.08), strength=0.9, shape='Smooth'), 0.1, 0.4, FLAT),
    track(step('WidgetPunch', scaleChange=vec2(0.06, 0.12), shape='Smooth'), 0.1, 0.4, FLAT),
    track(step('WidgetFlash', color=color(1, 0.12, 0.08), strength=0.6, shape='Smooth'), 0.55, 0.4, FLAT),
    track(step('WidgetPunch', scaleChange=vec2(0.04, 0.08), shape='Smooth'), 0.55, 0.4, FLAT),
], sustain=(0.1, 1.0))

recipe('HudRecovered', 'Reward', 'The sprint meter is full again after running empty: it brightens once.', [
    track(step('WidgetPunch', scaleChange=vec2(0.08, 0.2), shape='Kick', attackFraction=0.25), 0.0, 0.3, FLAT),
    track(step('WidgetFlash', color=color(0.85, 1, 0.9), strength=0.6, shape='Kick'), 0.0, 0.4, FLAT),
])

# Plays on the lamp itself, so the buzz comes from it and you can see it drop out.
recipe('LightUnease', 'Dread', 'Walking under a failing light: the lamp buzzes, surges and drops out for a moment, and the color goes cold.', [
    track(step('LightFlash', intensityChange=-0.85, bFlicker=True, flickerRate=11.0), 0.0, 1.2, FLAT),
    track(step('PlaySound', sound=SOUND.format('Light_Buzz'), placement='AtTargetLocation', attenuationSettings=ATT, volumeMultiplier=0.45, pitchMultiplier=0.85, pitchVariation=0.08), 0.0, 1.0, FLAT),
    track(step('PlaySound', sound=SOUND.format('Light_Crackle'), placement='AtTargetLocation', attenuationSettings=ATT, volumeMultiplier=0.5, pitchVariation=0.2), 0.05, 0.1, FLAT),
    track(step('ColorTint', tintColor=color(0.6, 0.75, 1.0), strength=0.3, shape='Smooth'), 0.0, 1.6, BELL),
    track(step('CameraRoll', rollDegrees=1.0, bRandomDirection=True), 0.0, 1.6, BELL),
], cooldown=8.0)

recipe('Doorway', 'Dread', 'Stepping through a doorway: the frame creaks somewhere above you.', [
    track(step('PlaySound', sound=SOUND.format('Door_Creak'), placement='AtTargetLocation', attenuationSettings=ATT, volumeMultiplier=0.4, pitchMultiplier=0.85, pitchVariation=0.15), 0.0, 0.5, FLAT),
], cooldown=6.0)

recipe('Land', 'Weight', 'Landing after a real drop: a soft thud and a small dip that grow with the fall. Stepping off a kerb stays silent.', [
    track(step('PlaySound', sound=SOUND.format('Land_Soft'), placement='TwoD', volumeMultiplier=0.6, pitchMultiplier=0.9, pitchVariation=0.08), 0.0, 0.25, FLAT,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.15, 0), (0.3, 0.7), (1, 1)])]),
    track(step('CameraPunch', locationPunch=vec(0, 0, -4), rotationPunch=rot(-1.2, 0, 0), shape='Kick', attackFraction=0.12), 0.0, 0.3,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.15, 0), (0.3, 0.5), (1, 1.2)])]),
], parameters=[LAND_SPEED])

recipe('Scare', 'Dread', 'A scare spot: the flashlight dies, the screen nearly goes black for a moment, a low boom, the heart jumps and the camera flinches back.', [
    track(step('LightFlash', intensityChange=-1.0, bFlicker=True, flickerRate=14.0), 0.0, 0.25, FLAT),
    track(step('LightFlash', intensityChange=-1.0), 0.25, 0.9, FLAT),
    track(step('PlaySound', sound=SOUND.format('Light_Crackle'), placement='TwoD', volumeMultiplier=0.8), 0.0, 0.1, FLAT),
    track(step('ScreenFade', fadeColor=color(0, 0, 0), maxOpacity=0.85, fadeInFraction=0.15, fadeOutFraction=0.45), 0.2, 1.2, FLAT),
    track(step('PlaySound', sound=SOUND.format('Scare_Boom'), placement='TwoD', volumeMultiplier=1.0), 0.2, 2.0, FLAT),
    track(step('CameraPunch', locationPunch=vec(-8, 0, 0), rotationPunch=rot(2.5, 0, 0), shape='Kick', attackFraction=0.08), 0.2, 0.6),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Kick'), 0.2, 0.35),
    track(step('PlaySound', sound=SOUND.format('Heartbeat'), placement='TwoD', volumeMultiplier=1.0, pitchMultiplier=1.3, bStopAtTrackEnd=True, fadeOutTime=0.8), 0.5, 3.0, FLAT),
    track(step('VignettePulse', vignetteIntensity=0.8, shape='Smooth', frequency=1.6, damping=0.4, repeats=4), 0.5, 3.0, FADE),
], cooldown=30.0)


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
