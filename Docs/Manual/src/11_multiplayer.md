# Multiplayer {#ch11}

Feedback is personal: each player sees their own screen, feels their own controller and has their own comfort settings. FeelKit therefore never sends effects over the network. It sends only the request to play, and every machine that receives it plays the recipe itself, with its own players' comfort. The sections below cover how requests travel with the Feel Replication component of FeelKit Pro, how far they reach, and how hitstops behave when several machines share one world.

## What travels over the network {#ch11_what}

**Play Feel** plays on the machine that calls it, and nowhere else. In a networked game, that is often all that is needed: code that already runs on every machine, such as a replicated hit event or an animation notify, can call **Play Feel** on each of them, and each plays its own copy.

When a play has to start on one machine and reach others, the Feel Replication component of FeelKit Pro sends it. What travels is the request: the recipe or event, the target, the intensity, and the play context (parameters, instigator, direction, location, normal and context tags). Each receiving machine then plays the recipe with its own comfort settings, so a player who turned camera shake off never gets shake from another player's action.

Camera, screen and controller effects go to the local player of the play's target on each machine ([Ref: ch02_targets]). A target that belongs to another machine's player sends none of them on this machine; that player's own machine shows them.

## The Feel Replication component and its modes {#ch11_component}
[edition: Pro]

Add a **Feel Replication** component to a replicated actor, such as a character or a weapon. It has two nodes:

- **Play Feel Networked** plays a recipe.
- **Send Feel Event Networked** sends a Feel Event; each machine picks the recipe from its own Feel Maps ([Ref: ch06_events]).

Both take the same inputs as their local counterparts, plus **Mode** and, under the node's advanced pins, **Relevancy Distance** ([see: ch11_relevancy]). Both return the handle of the play on the calling machine, which is invalid when that machine does not play it.

[shot: S11-01 | A character with a Feel Replication component plays HeavyHit through Play Feel Networked, with Mode Everyone]

**Mode** chooses which machines play:

| Mode | Plays on |
|---|---|
| **Everyone** | Every machine. |
| **Owner Only** | Only the machine that owns the component's actor, for feedback meant for one player. The actor must belong to a player: their pawn, their controller, or something they own. |
| **Skip Owner** | Every machine except the owner's, for feedback the owner has already played locally. In a single-player game nothing plays. |

**Where to call it.** The component routes the request by where it is called:

- **On the server**, the request goes to the machines the mode selects.
- **On the client that owns the actor**, that client plays at once, without waiting for the server, and the server forwards the play to the others. Feedback for the player's own action therefore has no network delay.
- **On any other client**, the play stays on that client.

**Targets.** The target must exist on every machine that plays: a replicated actor, or one placed in the level. A widget is local interface and is sent as no target.

**Reliability.** Requests are sent unreliably, like other cosmetic events. Under heavy packet loss, a remote machine can miss a play; the game's state never depends on one.

## Relevancy distance {#ch11_relevancy}
[edition: Pro]

**Relevancy Distance** keeps distant plays off machines that would not see them. A machine skips the play when all of its local players are farther than this from the play's target, measured from each player's character or camera, whichever is closer. A player's own character is always in range, and a play whose distance cannot be measured is played. 0, the default, means no limit.

For distance-dependent strength rather than a cut-off, use the **Distance** parameter, which each machine fills in with its own camera distance ([Ref: ch07_distance]).

## Hitstop and slow motion in networked games {#ch11_time}

In a networked game, world time is shared by every player on a machine, and on a listen server by every connected player. A global hitstop that slowed the whole world would freeze other players' games in the middle of their own actions, and a client's world time can be overridden by the server's.

So in networked games, **Global Hitstop** and **Slow-mo Ramp** slow only the play's target, on that machine, instead of the whole world. A single-player game always changes world time.

**Allow Global Time Dilation in Multiplayer**, under **Project Settings** > **Plugins** > **FeelKit** > **Playback**, changes world time anyway: on a listen server that slows every connected player, and on a client the server's time can override it. Use it for games where a shared freeze is intended, such as a cooperative finishing move. The setting is part of FeelKit Pro; in Lite, networked games always slow only the target.

[shot: S11-02 | The Playback settings, with Allow Global Time Dilation in Multiplayer]

## Dedicated servers {#ch11_server}

A dedicated server has no players to show feedback to, so FeelKit plays nothing there: **Play Feel** returns no play, and the Feel Replication component forwards requests without playing them. Code shared between server and clients can call FeelKit without checking where it runs.

**Testing.** Play In Editor with **Net Mode** set to **Play As Listen Server** or **Play As Client** and two or more players runs a networked game on one computer. The FeelKit Debugger shows each world as its own row, labeled **Listen server**, **Client** or **Dedicated server**, with the plays of each ([Ref: ch10_debugger]).

Split-screen, with several local players on one machine, has not been tested.
