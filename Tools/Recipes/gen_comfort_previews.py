"""Writes the comfort menu's preview recipes as JSON (Tools/Recipes/ComfortMenu/FR_Comfort_<Name>.json).

Each one shows one comfort group on its own, so a player feels what a slider changes; FR_Comfort_Try shows them all.
Only effects that Lite includes are used, because the menu ships in Lite and Pro.
Run: python gen_comfort_previews.py
"""
import json
import os
import shutil
from gen_library import step, track, vec, rot, color, curve, FLAT, FADE, SOUND

ROOT = r'B:\NewUE5Project\Tools\Recipes\ComfortMenu'
RECIPES = []


def recipe(name, description, tracks):
    RECIPES.append(('FR_Comfort_' + name, {
        'tracks': tracks,
        'cooldown': 0.0,
        'defaultIntensity': 1.0,
        'feeling': {'tagName': 'Feel.Feeling.Interface'},
        'description': description,
    }))


# Each call makes a fresh step, because track() consumes the step it is given.
SHAKE = lambda: step('ProceduralShake', frequency=14.0, rotationAmplitude=rot(1.2, 1.2, 0.6), locationAmplitude=vec(0, 4, 4))
PUNCH = lambda: step('CameraPunch', locationPunch=vec(-24, 0, -10), rotationPunch=rot(-3.0, 0, 0), shape='Kick', attackFraction=0.12)
FLASH = lambda: step('ScreenFlash', color=color(1, 1, 1), maxOpacity=0.5)
VIGNETTE = lambda: step('VignettePulse', vignetteIntensity=1.2, shape='Smooth')
RUMBLE = lambda: step('ForceFeedbackCurve', leftLarge=0.8, rightLarge=0.8, leftSmall=0.5, rightSmall=0.5, shape='Kick')
HITSTOP = lambda: step('GlobalHitstop', timeDilation=0.05)
SLOWMO = lambda: step('SlowMoRamp', timeDilation=0.35, rampInTime=0.05, rampOutTime=0.25)
ZOOM = lambda: step('FOVKick', fieldOfViewKick=10.0, shape='Kick', attackFraction=0.15)

recipe('CameraShake', 'Comfort menu preview of the Camera Shake setting: a short shake.', [track(SHAKE(), 0.0, 0.45, FADE)])
recipe('CameraMotion', 'Comfort menu preview of the Camera Motion setting: one push of the camera.', [track(PUNCH(), 0.0, 0.5, FLAT)])
recipe('Flashes', 'Comfort menu preview of the Flashes setting: one white flash.', [track(FLASH(), 0.0, 0.12, FADE)])
recipe('Hitstop', 'Comfort menu preview of the Hitstop and Slow-mo setting: a short freeze, then a moment of slow motion. Visible when the game behind the menu is running.', [
    track(HITSTOP(), 0.0, 0.1, FLAT),
    track(SLOWMO(), 0.1, 0.5, FLAT),
])
recipe('ScreenDistortion', 'Comfort menu preview of the Screen Distortion setting: the edges of the screen darken and clear.', [track(VIGNETTE(), 0.0, 0.6, FLAT)])
recipe('Haptics', 'Comfort menu preview of the controller vibration setting: one short rumble.', [track(RUMBLE(), 0.0, 0.25, FLAT)])
recipe('Zoom', 'Comfort menu preview of the zoom speed limit: a quick zoom out and back, slowed by the limit.', [track(ZOOM(), 0.0, 0.45, FLAT)])
recipe('Try', 'Comfort menu Try button: a sample of every comfort group at once, played with the player\'s current settings.', [
    track(HITSTOP(), 0.0, 0.06, FLAT),
    track(SHAKE(), 0.0, 0.4, FADE),
    track(PUNCH(), 0.0, 0.5, FLAT),
    track(FLASH(), 0.0, 0.1, FADE),
    track(VIGNETTE(), 0.0, 0.5, FLAT),
    track(RUMBLE(), 0.0, 0.25, FLAT),
    track(step('PlaySound', sound=SOUND.format('Hit_Heavy'), placement='TwoD', volumeMultiplier=0.7), 0.0, 0.4, FLAT),
])


def write():
    if os.path.isdir(ROOT):
        shutil.rmtree(ROOT)
    os.makedirs(ROOT)
    for name, body in RECIPES:
        with open(os.path.join(ROOT, name + '.json'), 'w', encoding='utf-8') as file:
            json.dump({'format': 'FeelKitRecipe', 'schemaVersion': 2, 'name': name, 'recipe': body}, file, indent='\t')
            file.write('\n')
    print('wrote', len(RECIPES), 'recipes to', ROOT)


if __name__ == '__main__':
    write()
