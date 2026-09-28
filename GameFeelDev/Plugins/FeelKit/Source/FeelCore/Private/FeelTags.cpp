// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTags.h"

namespace FeelTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Shake, "Feel.Camera.Shake", "Camera shake effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Motion, "Feel.Camera.Motion", "Camera punches, FOV kicks and other camera motion.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Screen_Flash, "Feel.Screen.Flash", "Full-screen flashes.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Actor_Transform, "Feel.Actor.Transform", "Target actor transform effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Actor_Material, "Feel.Actor.Material", "Target material parameter effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Time_Hitstop, "Feel.Time.Hitstop", "Global and per-actor hitstops.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Time_SlowMo, "Feel.Time.SlowMo", "Slow motion.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Screen_Distortion, "Feel.Screen.Distortion", "Vignette, chromatic aberration and other screen distortion.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Haptics, "Feel.Haptics", "Controller vibration and haptics.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio, "Feel.Audio", "Sounds played by recipes.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Meta_Event, "Feel.Meta.Event", "Blueprint events and other calls into game code.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Meta_Recipe, "Feel.Meta.Recipe", "Nested recipes and choices between steps.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Screen_Color, "Feel.Screen.Color", "Screen color changes such as desaturation and tints.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Screen_Fade, "Feel.Screen.Fade", "Screen fades.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Actor_Light, "Feel.Actor.Light", "Light intensity and color changes on the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Mix, "Feel.Audio.Mix", "Volume, pitch and filtering of sound classes.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI, "Feel.UI", "Widget effects and on-screen text.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Spawn, "Feel.Spawn", "Spawned effects such as decals and particles.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Impact, "Feel.Feeling.Impact", "Something connects: hits, bullets, collisions.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Weight, "Feel.Feeling.Weight", "Mass and gravity: landings, stomps, heavy objects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Power, "Feel.Feeling.Power", "Build-up and release: charges, blasts, abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Speed, "Feel.Feeling.Speed", "Fast movement: dashes, boosts, near misses.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Reward, "Feel.Feeling.Reward", "Good outcomes: pickups, streaks, progress.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Danger, "Feel.Feeling.Danger", "Harm and threat: damage, low health, alarms.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Dread, "Feel.Feeling.Dread", "Slow tension: unease, heartbeats, scares.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Denial, "Feel.Feeling.Denial", "Refusals: blocked actions, empty resources, wrong input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feeling_Interface, "Feel.Feeling.Interface", "Menus and HUD: hovers, presses, notifications, counters.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_Action, "Feel.Genre.Action", "Action and role-playing games.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_Shooter, "Feel.Genre.Shooter", "Shooters.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_Platformer, "Feel.Genre.Platformer", "Platformers.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_Horror, "Feel.Genre.Horror", "Horror games.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_UI, "Feel.Genre.UI", "Menus and interfaces of any game.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Genre_Vehicle, "Feel.Genre.Vehicle", "Driving and flying.");

	// FEELKIT_PRO_BEGIN
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_HitLanded, "Feel.Event.Hit.Landed", "An attack connected. Usually sent on the attacker, with the hit location.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_HitReceived, "Feel.Event.Hit.Received", "Something was hit. Usually sent on what was hit, with the attacker as instigator and the hit location and direction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Hurt, "Feel.Event.Hurt", "The player was hurt. Usually sent on the player, with the direction the damage came from.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Death, "Feel.Event.Death", "Something died or was destroyed. Usually sent on what died.");
	// FEELKIT_PRO_END
}
