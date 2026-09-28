"""Writes the Platformer demo kit recipes as JSON (Plugins/FeelKit/Demos/Platformer/FR_PLAT_<Name>.json).

Uses the helpers of gen_library.py. Run: python gen_demo_platformer.py
Deliberately strong first pass (like the Action/RPG kit), to be dialled down by feel.
"""
import json, os, shutil
from gen_library import step, track, mapping, parameter, color, vec, rot, curve, FLAT, FADE, BELL, SOUND

ROOT = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Demos\Platformer'

# Values come from the Feel Trigger component on the character (its events pass them as parameters).
LAND_SPEED = parameter('LandSpeed', 600.0, 300.0, 2000.0, 'Downward speed at landing in cm/s, passed by the Feel Trigger Landed event. A normal jump lands at about 600, a fall from a high platform at 1500 or more.')
JUMP_NUMBER = parameter('JumpNumber', 2.0, 2.0, 3.0, 'Which jump in the air this is (2 = double jump, 3 = triple jump), passed by the Feel Trigger Air Jumped event.')
LAUNCH_SPEED = parameter('LaunchSpeed', 1200.0, 500.0, 2000.0, 'Launch speed in cm/s, passed by the Feel Trigger Launched event. The template\'s wall jump launches at about 1200.')

SOFT_UP = [(0, 0), (0.08, 0), (0.18, 1), (1, 1)]  # a normal jump lands at about 0.2
HARD_ONLY = [(0, 0), (0.6, 0), (0.7, 1), (1, 1)]
BY_SPEED = [(0, 0.25), (1, 1.3)]

RECIPES = []


def recipe(name, feeling, description, tracks, parameters=None):
    body = {
        'tracks': tracks,
        'cooldown': 0.0,
        'defaultIntensity': 1.0,
        'feeling': {'tagName': 'Feel.Feeling.' + feeling},
        'genres': {'gameplayTags': [{'tagName': 'Feel.Genre.Platformer'}]},
        'description': description,
    }
    if parameters:
        body['parameters'] = parameters
    RECIPES.append(('FR_PLAT_' + name, body))


recipe('Jump', 'Speed', 'A jump from the ground: the body stretches up, a quick whoosh with a scuff of the feet, the camera lifts a little.', [
    track(step('SquashStretch', axis='Z', amount=0.22, shape='Kick', frequency=8.0, damping=7.0), 0.0, 0.3),
    track(step('PlaySound', sound=SOUND.format('Jump_Whoosh'), placement='AttachedToTarget', pitchVariation=0.08, volumeMultiplier=0.7), 0.0, 0.2, FLAT),
    track(step('PlaySound', sound=SOUND.format('Jump_Scuff'), placement='AttachedToTarget', pitchVariation=0.1, volumeMultiplier=0.6), 0.0, 0.15, FLAT),
    track(step('CameraPunch', locationPunch=vec(0, 0, 4), rotationPunch=rot(1.0, 0, 0), shape='Smooth'), 0.0, 0.35),
    track(step('ForceFeedbackCurve', leftSmall=0.3, rightSmall=0.3, shape='Kick'), 0.0, 0.08),
])

recipe('AirJump', 'Power', 'A jump in the air: a bigger stretch, a rising lift sound on top of the whoosh, the view widens and the camera pops up.', [
    track(step('SquashStretch', axis='Z', amount=0.3, shape='Kick', attackFraction=0.2), 0.0, 0.45),
    track(step('PlaySound', sound=SOUND.format('Air_Lift'), placement='AttachedToTarget', pitchVariation=0.05, volumeMultiplier=0.55), 0.0, 0.3, FLAT),
    track(step('PlaySound', sound=SOUND.format('Jump_Whoosh'), placement='AttachedToTarget', pitchMultiplier=1.25, pitchVariation=0.05, volumeMultiplier=0.8), 0.0, 0.2, FLAT),
    track(step('FOVKick', fieldOfViewKick=6.0, shape='Smooth'), 0.0, 0.5,
          parameterMappings=[mapping('JumpNumber', [(0, 1), (1, 1.5)])]),
    track(step('CameraPunch', locationPunch=vec(0, 0, 6), rotationPunch=rot(1.5, 0, 0), shape='Smooth'), 0.0, 0.45),
    track(step('ForceFeedbackCurve', leftSmall=0.45, rightSmall=0.45, leftLarge=0.15, rightLarge=0.15, shape='Kick'), 0.0, 0.12),
], parameters=[JUMP_NUMBER])

recipe('WallJump', 'Impact', 'A wall jump (any launch): a split-second grip on the wall, the camera thrown along the jump, a thump off the wall and a whoosh.', [
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.045, FLAT),
    track(step('CameraPunch', locationPunch=vec(14, 0, 0), rotationPunch=rot(3.0, 0, 0), directionSource='PlayDirection', shape='Kick', attackFraction=0.15), 0.0, 0.4,
          parameterMappings=[mapping('LaunchSpeed', [(0, 0.6), (1, 1.3)])]),
    track(step('SquashStretch', axis='Z', amount=-0.22, shape='Kick', attackFraction=0.15), 0.0, 0.35),
    track(step('PlaySound', sound=SOUND.format('Wall_Thump'), placement='AttachedToTarget', pitchVariation=0.08, volumeMultiplier=0.9), 0.0, 0.2, FLAT),
    track(step('PlaySound', sound=SOUND.format('Jump_Whoosh'), placement='AttachedToTarget', pitchMultiplier=0.9, pitchVariation=0.05), 0.03, 0.2, FLAT),
    track(step('ChromaticAberration', fringeIntensity=2.5), 0.0, 0.25),
    track(step('ForceFeedbackCurve', leftLarge=0.5, rightLarge=0.5, leftSmall=0.4, rightSmall=0.4, shape='Kick'), 0.0, 0.14),
], parameters=[LAUNCH_SPEED])

recipe('Dash', 'Speed', 'A dash: the view stretches wide, color fringes at the edges, the camera lags behind, a deep whoosh and a rumble.', [
    track(step('FOVKick', fieldOfViewKick=12.0, shape='Kick', attackFraction=0.15), 0.0, 0.55),
    track(step('ChromaticAberration', fringeIntensity=4.0), 0.0, 0.35, FADE),
    track(step('VignettePulse', vignetteIntensity=0.35, shape='Smooth'), 0.0, 0.45, BELL),
    track(step('CameraPunch', locationPunch=vec(-14, 0, 0), rotationPunch=rot(0, 0, 0), shape='Kick'), 0.0, 0.4),
    track(step('PlaySound', sound=SOUND.format('Dash_Whoosh'), placement='AttachedToTarget', pitchMultiplier=0.85, pitchVariation=0.05), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.45, rightLarge=0.45, leftSmall=0.6, rightSmall=0.6, shape='Smooth'), 0.0, 0.3),
])

recipe('Land', 'Weight', 'Landing, scaled by the fall: a light step after a hop, a squash, a dip of the view and a thud after a real jump, a deep drop of the view with a short freeze after a long fall. The view only ever goes down and settles back slowly, like knees taking the weight.', [
    track(step('SquashStretch', axis='Z', amount=-0.3, shape='Spring', frequency=6.0, damping=9.0), 0.0, 0.45,
          parameterMappings=[mapping('LandSpeed', BY_SPEED)]),
    # The view drops fast (about 50 ms) and settles back over the rest of the track. Flat intensity, so the settle is the
    # Kick's own eased return and never a quick rebound. No shake: it would move the view up as much as down.
    track(step('CameraPunch', locationPunch=vec(0, 0, -8), rotationPunch=rot(-2.2, 0, 0), shape='Kick', attackFraction=0.09), 0.0, 0.55, FLAT,
          parameterMappings=[mapping('LandSpeed', [(0, 0.15), (0.2, 0.45), (0.7, 1.0), (1, 1.1)])]),
    # Hard landings add a deeper, longer drop on top, growing from medium falls to long ones.
    track(step('CameraPunch', locationPunch=vec(0, 0, -10), rotationPunch=rot(-3.0, 0, 0), shape='Kick', attackFraction=0.06), 0.0, 0.85, FLAT,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.35, 0), (0.7, 0.7), (1, 1)])]),
    track(step('GlobalHitstop', timeDilation=0.05), 0.0, 0.05, FLAT, parameterMappings=[mapping('LandSpeed', HARD_ONLY)]),
    track(step('PlaySound', sound=SOUND.format('Land_Step'), placement='AttachedToTarget', pitchVariation=0.1, volumeMultiplier=0.8), 0.0, 0.15, FLAT),
    track(step('PlaySound', sound=SOUND.format('Land_Soft'), placement='AttachedToTarget', pitchVariation=0.08), 0.0, 0.2, FLAT,
          parameterMappings=[mapping('LandSpeed', SOFT_UP)]),
    track(step('PlaySound', sound=SOUND.format('Land_Heavy'), placement='AttachedToTarget', pitchVariation=0.05), 0.0, 0.6, FLAT,
          parameterMappings=[mapping('LandSpeed', HARD_ONLY)]),
    track(step('ForceFeedbackCurve', leftLarge=0.8, rightLarge=0.8, leftSmall=0.5, rightSmall=0.5, shape='Kick'), 0.0, 0.2,
          parameterMappings=[mapping('LandSpeed', [(0, 0.1), (1, 1)])]),
], parameters=[LAND_SPEED])


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
