# Glossary {#appA}

{widths: 24,76}
| Term | Meaning |
|---|---|
| Accumulator | A named value, defined in the project settings, that builds up when the game adds to it and falls back toward 0 over time. Recipe parameters can read it. [See: ch02_parameters]. |
| Additive Capped | Camera arbitration mode in which the camera output of separate plays adds up and the sum is limited by caps. [See: set_camera]. |
| Applies To | Track field that decides whether the track's actor effects go to the play's target or to its instigator. [See: ch02_targets]. |
| Blend out | Fading a play out over **Blend Out Time** (0.2 s by default) when it is stopped with **Stop Feel** and Blend Out on. |
| Channel | Gameplay tag under `Feel` on every track, such as `Feel.Camera.Shake`. It decides the track's comfort group and its color on the timeline. [See: ch02_channels]. |
| Comfort group | One of the six groups a player can scale separately: Camera Shake, Camera Motion, Flashes, Hitstop and Slow-mo, Screen Distortion and Haptics. [See: ch02_comfort]. |
| Comfort preset | A full set of comfort scales applied at once: the three built-in presets or a Feel Comfort Preset asset. |
| Comfort scale | A player's setting for Master or one comfort group, from 0 (off) to 1 (full strength). |
| Context tags | Gameplay tags passed with a play that describe its circumstances. Feel Maps use them to choose between rows. |
| Cooldown | Recipe field: seconds before the recipe can play again on the same target. |
| Default channel | The channel a new track of a step gets. Listed for each step in [ref: ch16]. |
| Default Intensity | Recipe field that multiplies every play of the recipe. |
| Essential track | A track that carries information the player needs. When its group is turned off, its substitute step plays, or it keeps its essential floor. |
| Essential Floor | Track field: the lowest comfort scale an essential track without a substitute can drop to. It does not apply on Camera Shake and Camera Motion. |
| Feel Event | A gameplay tag sent with **Send Feel Event** that says what happened. A Feel Map picks the recipe. [See: ch02_events]. |
| Feel Input | Component that plays recipes or sends Feel Events from Enhanced Input actions. [See: ref_nodes_input]. |
| Feel Map | Asset whose rows assign recipes to Feel Events. [See: ref_feel_map]. |
| Feel Replication | Component that plays recipes and sends Feel Events on several machines in a networked game. [See: ref_nodes_replication]. |
| Feel Switch | Actor that lets players turn FeelKit off and on while playing, to compare. [See: ref_feel_switch]. |
| Feel Trigger | Component that plays recipes or sends Feel Events when common events happen to its owner, such as landing or taking damage. [See: ref_nodes_trigger]. |
| Flash limiter | Part of each player's comfort settings that limits how many flash tracks may start per second. |
| Handle | The value a play node returns, used to stop the play, release it or change its parameters. It stays safe to keep after the play ends. |
| Instant track | A track with a length of 0, for steps that act once, such as Play Sound. |
| Instigator | A second actor passed with a play, besides the target, such as the attacker of a hit. [See: ch02_targets]. |
| Intensity | How strongly a track plays at a moment: call intensity, recipe default, intensity curve, parameter mappings, random intensity and comfort multiplied together. [See: ch02_intensity]. |
| Intensity curve | Track field: the intensity over the track, from its start to its end. |
| Library recipe | A ready-made recipe that ships with FeelKit under /FeelKit/Library. It is read-only; copy it into your project to change it. |
| Max Concurrent | Recipe field: how many plays of the recipe can run at once on one target. 0 means no limit. |
| Motion shape | The form of a punch-like motion over a track: Spring, Kick or Smooth. [See: ref_motion_settings]. |
| No preview | Label on tracks whose step the editor preview cannot show, such as hitstops and controller vibration. They still play in the game. |
| Parameter | A named number a recipe accepts each time it plays, such as Damage. [See: ch02_parameters]. |
| Parameter mapping | Track field: a curve that turns a parameter into an intensity multiplier for the track. |
| Play | One running instance of a recipe, started by a play node and identified by its handle. |
| Play context | The optional information passed with a play: parameter values, instigator, direction, location, normal and context tags. [See: ref_play_context]. |
| Play in PIE | Recipe editor button that plays the open recipe on the player's pawn in a running Play In Editor session. |
| Priority | Field of the time steps. When time requests overlap on one clock, the highest priority wins. |
| Real time | Time that hitstops and slow motion do not slow. Recipes are timed in real time. |
| Recipe | Asset that describes how one moment feels, as timed tracks of steps. [See: ch02_recipes]. |
| Release | Ending the sustain loop of a sustained play so that it plays the rest of the recipe: by the game, or by the recipe's own Release Parameter. |
| Seed | Track field that fixes the noise and random choices of the step, so the same seed gives the same result. |
| Step | The effect a track plays, such as Camera Punch or Play Sound. [See: ch16]. |
| Strongest Wins | The default camera arbitration mode: for location, rotation and field of view, the strongest play wins. [See: set_camera]. |
| Substitute step | Track field: the step an essential track plays instead when the player turns its comfort group off. |
| Sustain | A looping region of a recipe that repeats until the play is released. [See: ch02_parameters]. |
| Target | What a recipe plays on: an actor, a component, a location, a player's camera or a widget. [See: ch02_targets]. |
| Track | One timed step inside a recipe, with its own start time, length, channel and intensity curve. |
