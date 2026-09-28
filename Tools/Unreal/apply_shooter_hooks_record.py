import os, re
ROOT = r'B:\NewUE5Project\FeelDemoFP\Source\FeelDemoFP'


def edit(rel, pairs):
    p = os.path.join(ROOT, rel)
    s = open(p, encoding='utf-8').read()
    crlf = '\r\n' in s
    s = s.replace('\r\n', '\n')
    for old, new in pairs:
        assert old in s, (rel, old[:80])
        s = s.replace(old, new, 1)
    if crlf:
        s = s.replace('\n', '\r\n')
    open(p, 'w', encoding='utf-8', newline='').write(s)


edit('Variant_Shooter/AI/ShooterNPC.cpp', [
    ('#include "Variant_Shooter/AI/ShooterNPC.h"\n', '#include "Variant_Shooter/AI/ShooterNPC.h"\n#include "FeelBlueprintLibrary.h"\n'),
    ('''	// Reduce HP
	CurrentHP -= Damage;
''', '''	// Reduce HP
	CurrentHP -= Damage;

	// FeelKit: hit or kill confirmation for the player who dealt the damage
	APawn* Attacker = EventInstigator ? EventInstigator->GetPawn() : nullptr;
	UFeelRecipe* Feel = CurrentHP <= 0.0f ? KillFeel.Get() : HitFeel.Get();
	if (Feel && Attacker && Attacker->IsPlayerControlled())
	{
		FFeelPlayContext Context;
		Context.Instigator = this;
		Context.Location = GetActorLocation();
		Context.Parameters.Add(TEXT("Damage"), Damage);
		UFeelBlueprintLibrary::PlayFeelWithContext(this, Feel, FFeelTarget::FromActor(Attacker), Context);
	}
'''),
])

# Player.
edit('Variant_Shooter/ShooterCharacter.h', [
    ('class AShooterWeapon;\n', 'class AShooterWeapon;\nclass UFeelRecipe;\n'),
    ('''	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;
''', '''	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	/** FeelKit: recipe played on this character when it takes damage, pointing from the damage source */
	UPROPERTY(EditAnywhere, Category="FeelKit")
	TObjectPtr<UFeelRecipe> HurtFeel;

	/** FeelKit: recipe played on this character when it dies */
	UPROPERTY(EditAnywhere, Category="FeelKit")
	TObjectPtr<UFeelRecipe> DeathFeel;
'''),
])
edit('Variant_Shooter/ShooterCharacter.cpp', [
    ('#include "ShooterCharacter.h"\n', '#include "ShooterCharacter.h"\n#include "FeelBlueprintLibrary.h"\n'),
    ('''	// Reduce HP
	CurrentHP -= Damage;
''', '''	// Reduce HP
	CurrentHP -= Damage;

	// FeelKit: being hit, pointing from the damage source
	if (HurtFeel && CurrentHP > 0.0f)
	{
		FFeelPlayContext Context;
		Context.Instigator = DamageCauser;
		Context.Location = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
		Context.Direction = (GetActorLocation() - Context.Location).GetSafeNormal();
		Context.Parameters.Add(TEXT("Damage"), Damage);
		UFeelBlueprintLibrary::PlayFeelWithContext(this, HurtFeel, FFeelTarget::FromActor(this), Context);
	}
'''),
    ('''	// call the BP handler
	BP_OnDeath();
''', '''	// FeelKit: death
	if (DeathFeel)
	{
		UFeelBlueprintLibrary::PlayFeel(this, DeathFeel, FFeelTarget::FromActor(this));
	}

	// call the BP handler
	BP_OnDeath();
'''),
])
print('ok')

# 2026-09-22 (after the first play test): AShooterCharacter also got PickupFeel (start of AddWeaponClass),
# SwitchWeaponFeel (end of DoSwitchWeapon) and NoWeaponFeel (DoStartFiring without a weapon). Applied by hand.
