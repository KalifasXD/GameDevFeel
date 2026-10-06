"""Writes the Shooter demo kit recipes as JSON (Plugins/FeelKit/Demos/Shooter/FR_SHOOT_<Name>.json).

Uses the helpers of gen_library.py. Run: python gen_demo_shooter.py
Deliberately strong first pass, to be dialled down by feel. Movement-safe shapes (Kick / Smooth) for camera motion.
"""
import json, os, shutil
from gen_library import step, track, mapping, parameter, color, vec, rot, curve, FLAT, FADE, BELL, SOUND

ROOT = r'B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Demos\Shooter'
ATT = '/FeelKit/Samples/Sounds/ATT_FK_World.ATT_FK_World'
HIT_FLASH = '/FeelKit/Samples/Materials/M_FK_HitFlash.M_FK_HitFlash'
SCORCH = '/FeelKit/Samples/Materials/M_FK_Decal_Scorch.M_FK_Decal_Scorch'

HEAT = parameter('Heat', 0.0, 0.0, 8.0, 'How hot the gun runs: every shot adds 1 to the ShotHeat accumulator of the shooter, which cools down quickly once firing stops. Sustained fire shakes more.', accumulator='ShotHeat')
DISTANCE = parameter('Distance', 1000.0, 0.0, 3000.0, 'Distance from the player to the explosion in cm, passed by the projectile hook. Close explosions hit hard, far ones only rumble.')
LAND_SPEED = parameter('LandSpeed', 600.0, 300.0, 2000.0, 'Downward speed at landing in cm/s, passed by the Feel Trigger Landed event. Walking off a step lands well under 600; a jump pad arc comes down at 1000 or more.')
DAMAGE = parameter('Damage', 25.0, 0.0, 100.0, 'Damage of the hit, passed by the hook. Shown as a number over the enemy.')
HURT_DAMAGE = parameter('Damage', 25.0, 0.0, 100.0, 'Damage the player took, passed by the hook. At 25, the default damage of the template projectile, the recipe plays as tuned; harder hits knock and pulse harder.')
# Hurt strength by damage: 25 is the tuned feel, 0 about two thirds of it, 100 two fifths more.
HURT_SCALE = [(0, 0.65), (0.25, 1), (1, 1.4)]

CLOSE_ONLY = [(0, 1), (0.25, 1), (0.3, 0), (1, 0)]
FALLOFF = [(0, 1), (0.5, 0.35), (1, 0)]

RECIPES = []


def recipe(name, feeling, description, tracks, parameters=None):
    body = {
        'tracks': tracks,
        'cooldown': 0.0,
        'defaultIntensity': 1.0,
        'feeling': {'tagName': 'Feel.Feeling.' + feeling},
        'genres': {'gameplayTags': [{'tagName': 'Feel.Genre.Shooter'}]},
        'description': description,
    }
    if parameters:
        body['parameters'] = parameters
    RECIPES.append(('FR_SHOOT_' + name, body))


def shot(name, volume, pitch=1.0):
    return step('PlaySound', sound=SOUND.format(name), placement='TwoD', volumeMultiplier=volume, pitchMultiplier=pitch, pitchVariation=0.05)


recipe('Pistol', 'Power', 'A pistol shot from the player: a sharp crack over a low thump, the view kicks up and zooms in a touch, a short rumble.', [
    track(shot('Shot_Pistol', 0.8), 0.0, 0.3, FLAT),
    track(shot('Shot_Thump', 0.5), 0.0, 0.3, FLAT),
    track(step('CameraPunch', locationPunch=vec(-6, 0, -1), rotationPunch=rot(1.8, 0, 0), shape='Kick', attackFraction=0.1, directionJitter=10.0), 0.0, 0.3),
    track(step('FOVKick', fieldOfViewKick=-2.0, shape='Kick', attackFraction=0.1), 0.0, 0.25),
    track(step('ForceFeedbackCurve', rightSmall=0.6, leftSmall=0.2, shape='Kick'), 0.0, 0.08),
])

recipe('Rifle', 'Power', 'A rifle shot from the player, fired in bursts: a small kick per shot and a shake that builds up the longer the trigger is held.', [
    track(shot('Shot_Rifle', 0.7), 0.0, 0.25, FLAT),
    track(shot('Shot_Thump', 0.3), 0.0, 0.25, FLAT),
    track(step('CameraPunch', locationPunch=vec(-3, 0, 0), rotationPunch=rot(0.8, 0, 0), shape='Kick', attackFraction=0.1, directionJitter=15.0), 0.0, 0.15),
    track(step('ProceduralShake', frequency=22.0, rotationAmplitude=rot(0.8, 0.6, 0.3), locationAmplitude=vec(0, 0.5, 0.5)), 0.0, 0.18,
          parameterMappings=[mapping('Heat', [(0, 0.1), (1, 1)])]),
    track(step('ForceFeedbackCurve', rightSmall=0.45, leftSmall=0.2, shape='Kick'), 0.0, 0.06,
          parameterMappings=[mapping('Heat', [(0, 0.5), (1, 1)])]),
], parameters=[HEAT])

recipe('Launcher', 'Power', 'A grenade launch from the player: a deep boom, a heavy kick up and back, the view widens, a strong rumble.', [
    track(shot('Shot_Launcher', 1.0), 0.0, 0.7, FLAT),
    track(shot('Shot_Thump', 0.8, 0.7), 0.0, 0.4, FLAT),
    track(step('CameraPunch', locationPunch=vec(-14, 0, -3), rotationPunch=rot(4.5, 0, 0), shape='Kick', attackFraction=0.12), 0.0, 0.5),
    track(step('FOVKick', fieldOfViewKick=5.0, shape='Kick', attackFraction=0.15), 0.0, 0.4),
    track(step('ChromaticAberration', fringeIntensity=2.0), 0.0, 0.25),
    track(step('ForceFeedbackCurve', leftLarge=0.8, rightLarge=0.8, leftSmall=0.4, rightSmall=0.4, shape='Kick'), 0.0, 0.2),
])

recipe('EnemyFire', 'Danger', 'Anyone else firing, such as an enemy: the shot sounds from the shooter and fades with distance. No camera effects.', [
    track(step('PlaySound', sound=SOUND.format('Shot_Rifle'), placement='AttachedToTarget', attenuationSettings=ATT, volumeMultiplier=0.8, pitchMultiplier=0.85, pitchVariation=0.06), 0.0, 0.25, FLAT),
])

recipe('Impact', 'Impact', 'A bullet hits the world: a metallic ping over a dull thud where it lands, and a scorch mark that fades.', [
    track(step('PlaySound', sound=SOUND.format('Impact_Ping'), placement='AtTargetLocation', attenuationSettings=ATT, volumeMultiplier=0.6, pitchVariation=0.15), 0.0, 0.25, FLAT),
    track(step('PlaySound', sound=SOUND.format('Impact_Thud'), placement='AtTargetLocation', attenuationSettings=ATT, volumeMultiplier=0.4, pitchVariation=0.1), 0.0, 0.4, FLAT),
    track(step('SpawnDecal', decalMaterial=SCORCH, decalSize=vec(8, 14, 14), lifetime=6.0, fadeOutTime=1.5), 0.0, 0.05, FLAT),
])

recipe('Hit', 'Impact', 'The player hits an enemy: a crisp tick, the enemy flashes white and freezes for a moment, the damage pops up over it.', [
    track(step('PlaySound', sound=SOUND.format('Hit_Tick'), placement='TwoD', volumeMultiplier=0.8, pitchMultiplier=1.1, pitchVariation=0.05), 0.0, 0.1, FLAT),
    track(step('HitFlash', flashMaterial=HIT_FLASH, color=color(1, 1, 1), shape='Kick'), 0.0, 0.12, FLAT, appliesTo='Instigator'),
    track(step('ActorHitstop', timeDilation=0.02), 0.0, 0.06, FLAT, appliesTo='Instigator'),
    track(step('NumberPop', valueParameter='Damage', color=color(1, 1, 1), fontSize=26.0, popScale=1.5, riseDistance=70.0, worldOffset=vec(0, 0, 90)), 0.0, 0.7, FLAT),
    track(step('ForceFeedbackCurve', rightSmall=0.25, shape='Kick'), 0.0, 0.05),
], parameters=[DAMAGE])

recipe('Kill', 'Reward', 'The player takes an enemy down: a confirmation chime, a red flash on the enemy, a brief freeze into slow motion, a small zoom and a rumble.', [
    track(step('PlaySound', sound=SOUND.format('Kill_Confirm'), placement='TwoD', volumeMultiplier=0.8), 0.0, 0.4, FLAT),
    track(step('PlaySound', sound=SOUND.format('Hit_Tick'), placement='TwoD', volumeMultiplier=0.8, pitchMultiplier=0.8), 0.0, 0.1, FLAT),
    track(step('HitFlash', flashMaterial=HIT_FLASH, color=color(1, 0.3, 0.2), shape='Kick'), 0.0, 0.25, FLAT, appliesTo='Instigator'),
    track(step('GlobalHitstop', timeDilation=0.03), 0.0, 0.05, FLAT),
    track(step('SlowMoRamp', timeDilation=0.35, rampInTime=0.03, rampOutTime=0.25), 0.05, 0.4, FLAT),
    track(step('CameraZoom', fieldOfViewChange=-5.0, easeInFraction=0.15, easeOutFraction=0.6), 0.0, 0.45, FLAT),
    track(step('VignettePulse', vignetteIntensity=0.3, shape='Smooth'), 0.0, 0.45, BELL),
    track(step('ForceFeedbackCurve', leftLarge=0.6, rightLarge=0.6, shape='Kick'), 0.0, 0.25),
])

recipe('Explosion', 'Impact', 'A grenade explodes, scaled by the player\'s distance: a crunch over a low boom, a hard shake, and up close a freeze, a warm flash and color fringes; far away only a rumble.', [
    track(step('PlaySound', sound=SOUND.format('Explosion_Crunch'), placement='AtTargetLocation', volumeMultiplier=1.0), 0.0, 0.8, FLAT,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0.4)])]),
    track(step('PlaySound', sound=SOUND.format('Explosion_Low'), placement='AtTargetLocation', volumeMultiplier=0.9), 0.0, 1.0, FLAT,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0.5)])]),
    track(step('ProceduralShake', frequency=18.0, rotationAmplitude=rot(3.0, 3.0, 1.5), locationAmplitude=vec(0, 6, 6)), 0.0, 0.6,
          parameterMappings=[mapping('Distance', FALLOFF)]),
    track(step('CameraPunch', locationPunch=vec(-10, 0, -6), rotationPunch=rot(-2.0, 0, 0), shape='Kick', attackFraction=0.1), 0.0, 0.5,
          parameterMappings=[mapping('Distance', FALLOFF)]),
    track(step('GlobalHitstop', timeDilation=0.03), 0.0, 0.04, FLAT, parameterMappings=[mapping('Distance', CLOSE_ONLY)]),
    track(step('ScreenFlash', color=color(1, 0.85, 0.6), maxOpacity=0.35), 0.0, 0.08, FLAT, parameterMappings=[mapping('Distance', CLOSE_ONLY)]),
    track(step('ChromaticAberration', fringeIntensity=3.0), 0.0, 0.4, parameterMappings=[mapping('Distance', CLOSE_ONLY)]),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, leftSmall=0.5, rightSmall=0.5, shape='Smooth'), 0.0, 0.5,
          parameterMappings=[mapping('Distance', [(0, 1), (1, 0.15)])]),
], parameters=[DISTANCE])

recipe('Hurt', 'Danger', 'The player is hit: the view is knocked away from the shot, a red pulse at the edges, a body hit and a rumble, all stronger for harder hits.', [
    track(step('CameraPunch', locationPunch=vec(8, 0, 0), rotationPunch=rot(2.0, 0, 0), directionSource='PlayDirection', shape='Kick', attackFraction=0.1), 0.0, 0.35,
          parameterMappings=[mapping('Damage', HURT_SCALE)]),
    track(step('ColorTint', tintColor=color(1, 0.2, 0.15), strength=0.5, shape='Kick', attackFraction=0.1), 0.0, 0.5,
          parameterMappings=[mapping('Damage', HURT_SCALE)]),
    track(step('VignettePulse', vignetteIntensity=0.6, shape='Kick', attackFraction=0.1), 0.0, 0.5,
          parameterMappings=[mapping('Damage', HURT_SCALE)]),
    track(step('PlaySound', sound=SOUND.format('Body_Hit'), placement='TwoD', volumeMultiplier=0.7, pitchVariation=0.08), 0.0, 0.3, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=0.6, rightLarge=0.3, shape='Kick'), 0.0, 0.2,
          parameterMappings=[mapping('Damage', HURT_SCALE)]),
], parameters=[HURT_DAMAGE])

recipe('Death', 'Danger', 'The player dies: time slows, the color drains, the view closes in, a heavy fall and a long rumble.', [
    track(step('SlowMoRamp', timeDilation=0.3, rampInTime=0.1, rampOutTime=0.5), 0.0, 1.4, FLAT),
    track(step('Desaturate', amount=1.0, shape='Smooth'), 0.0, 2.0, BELL),
    track(step('VignettePulse', vignetteIntensity=0.8, shape='Smooth'), 0.0, 2.0, BELL),
    track(step('CameraZoom', fieldOfViewChange=-8.0, easeInFraction=0.3, easeOutFraction=0.5), 0.0, 1.6, FLAT),
    track(step('PlaySound', sound=SOUND.format('Body_Fall'), placement='TwoD'), 0.1, 0.8, FLAT),
    track(step('ForceFeedbackCurve', leftLarge=1.0, rightLarge=1.0, shape='Smooth'), 0.0, 0.6),
])


recipe('Pickup', 'Reward', 'The player walks over a weapon pickup: a short pickup chime and a light tap on the controller.', [
    track(step('PlaySound', sound=SOUND.format('Pickup'), placement='TwoD', volumeMultiplier=0.6, pitchVariation=0.05), 0.0, 0.4, FLAT),
    track(step('ForceFeedbackCurve', rightSmall=0.2, shape='Kick'), 0.0, 0.06),
])

recipe('Switch', 'Weight', 'The player switches weapons: a metal latch and a slight dip of the view.', [
    track(step('PlaySound', sound=SOUND.format('Weapon_Switch'), placement='TwoD', volumeMultiplier=0.6, pitchVariation=0.05), 0.0, 0.3, FLAT),
    track(step('CameraPunch', locationPunch=vec(0, 0, -1.5), rotationPunch=rot(-0.6, 0, 0), shape='Smooth'), 0.0, 0.25),
])

recipe('DryFire', 'Denial', 'The player pulls the trigger with nothing in hand: a dull click, nothing more.', [
    track(step('PlaySound', sound=SOUND.format('Dry_Click'), placement='TwoD', volumeMultiplier=0.5, pitchVariation=0.08), 0.0, 0.2, FLAT),
])

recipe('JumpPad', 'Speed', 'A player steps on a jump pad: a rising whoosh, the view widens as you shoot up and the camera is pressed down for a moment, a smooth rumble.', [
    track(step('PlaySound', sound=SOUND.format('Air_Lift'), placement='AttachedToTarget', volumeMultiplier=0.6, pitchMultiplier=0.8, pitchVariation=0.05), 0.0, 0.4, FLAT),
    track(step('PlaySound', sound=SOUND.format('Dash_Whoosh'), placement='AttachedToTarget', volumeMultiplier=0.8, pitchMultiplier=0.8, pitchVariation=0.05), 0.0, 0.35, FLAT),
    track(step('FOVKick', fieldOfViewKick=7.0, shape='Smooth'), 0.0, 0.7),
    track(step('CameraPunch', locationPunch=vec(0, 0, -5), rotationPunch=rot(-1.5, 0, 0), shape='Kick', attackFraction=0.12), 0.0, 0.4),
    track(step('ForceFeedbackCurve', leftLarge=0.35, rightLarge=0.35, leftSmall=0.3, rightSmall=0.3, shape='Smooth'), 0.0, 0.3),
])

recipe('Land', 'Weight', 'The player lands from a real drop, such as after a jump pad: a camera dip and a thud that grow with the fall. Small steps stay silent.', [
    track(step('CameraPunch', locationPunch=vec(0, 0, -7), rotationPunch=rot(-2.0, 0, 0), shape='Kick', attackFraction=0.12), 0.0, 0.35,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.25, 0), (0.35, 0.6), (1, 1.2)])]),
    track(step('PlaySound', sound=SOUND.format('Land_Soft'), placement='TwoD', volumeMultiplier=0.8, pitchVariation=0.08), 0.0, 0.2, FLAT,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.25, 0), (0.35, 1), (1, 1)])]),
    track(step('PlaySound', sound=SOUND.format('Land_Heavy'), placement='TwoD', volumeMultiplier=0.8, pitchVariation=0.05), 0.0, 0.6, FLAT,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.6, 0), (0.7, 1), (1, 1)])]),
    track(step('ForceFeedbackCurve', leftLarge=0.6, rightLarge=0.6, shape='Kick'), 0.0, 0.18,
          parameterMappings=[mapping('LandSpeed', [(0, 0), (0.25, 0), (0.35, 0.4), (1, 1)])]),
], parameters=[LAND_SPEED])

# HUD (D-064): the template's bullet counter and score react. They play on widgets, so only widget steps and sound.
def vec2(x, y):
    return {'x': float(x), 'y': float(y)}


AMMO = parameter('Ammo', 1.0, 0.0, 1.0, 'Share of the magazine left after the shot, 0 to 1. Passed by the bullet counter.')
PLAYER_SCORED = parameter('PlayerScored', 1.0, 0.0, 1.0, '1 when the point is the player\'s (another team lost someone), 0 when the player\'s own team did. Passed by the score display.')
LOW_AMMO = [(0, 1), (0.25, 1), (0.3, 0), (1, 0)]
PLAYERS = [(0, 0), (0.5, 0), (0.55, 1), (1, 1)]
THEIRS = [(0, 1), (0.45, 1), (0.5, 0), (1, 0)]

recipe('HudShot', 'Interface', 'One shot fired: the bullet counter kicks down a little; when the magazine runs low it flashes red with every shot.', [
    track(step('WidgetPunch', scaleChange=vec2(0.04, 0.08), translation=vec2(0.0, 4.0), shape='Kick', attackFraction=0.2), 0.0, 0.12, FLAT),
    track(step('WidgetFlash', color=color(1, 0.2, 0.15), strength=0.9, shape='Kick'), 0.0, 0.18, FLAT,
          parameterMappings=[mapping('Ammo', LOW_AMMO)]),
], parameters=[AMMO])

recipe('HudEmpty', 'Denial', 'The magazine ran dry: the bullet counter shakes and burns red.', [
    track(step('WidgetShake', amplitude=vec2(10.0, 2.0), frequency=30.0, decay=1.5), 0.0, 0.4, FLAT),
    track(step('WidgetFlash', color=color(1, 0.15, 0.1), strength=1.0, shape='Kick'), 0.0, 0.45, FLAT),
])

recipe('HudReload', 'Reward', 'The magazine is full again: the bullet counter swells once and brightens, with a soft click.', [
    track(step('WidgetPunch', scaleChange=vec2(0.15, 0.25), shape='Kick', attackFraction=0.25), 0.0, 0.25, FLAT),
    track(step('WidgetFlash', color=color(0.7, 1, 0.8), strength=0.6, shape='Kick'), 0.0, 0.3, FLAT),
    track(step('PlaySound', sound=SOUND.format('UI_Tick'), placement='TwoD', volumeMultiplier=0.5, pitchMultiplier=0.9), 0.0, 0.15, FLAT),
])

recipe('HudHurt', 'Danger', 'Life lost: the life bar shakes and flashes red.', [
    track(step('WidgetShake', amplitude=vec2(8.0, 3.0), frequency=28.0, decay=1.4), 0.0, 0.35, FLAT),
    track(step('WidgetFlash', color=color(1, 0.15, 0.1), strength=0.9, shape='Kick'), 0.0, 0.35, FLAT),
])

recipe('HudScore', 'Reward', 'A score changes: the number jumps and lands; gold when the point is the player\'s, red when the player\'s team lost someone.', [
    track(step('WidgetPunch', scaleChange=vec2(0.5, 0.5), shape='Kick', attackFraction=0.15), 0.0, 0.35, FLAT),
    track(step('WidgetFlash', color=color(1, 0.8, 0.3), strength=1.0, shape='Kick'), 0.0, 0.5, FLAT,
          parameterMappings=[mapping('PlayerScored', PLAYERS)]),
    track(step('WidgetFlash', color=color(1, 0.2, 0.15), strength=1.0, shape='Kick'), 0.0, 0.5, FLAT,
          parameterMappings=[mapping('PlayerScored', THEIRS)]),
], parameters=[PLAYER_SCORED])


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
