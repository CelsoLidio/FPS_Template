// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Criando Camera FPS
	CameraFPS = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFPS"));
	CameraFPS->SetupAttachment(GetCapsuleComponent());
	CameraFPS->bUsePawnControlRotation = true;
	CameraFPS->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));

	//Init Variables//
	lookSensibility = 70.0f;
	isSprint = false;
	isJumping = false;
	normalSpeed = 600.0f;
	sprintSpeed = 1000.0f;	
	
	crouchEyeOffSet = FVector::Zero();
	crouchSpeed = 100.0f;
	maxHealth = 100.0f;

	if (GetMovementComponent())
	{
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();


	TArray<USceneComponent*> allChildCamera;

	CameraFPS->GetChildrenComponents(false, allChildCamera);
	GetCharacterMovement()->MaxWalkSpeed = normalSpeed;
	currentHealth = maxHealth;
}



// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float crouchTime = FMath::Min(1.0f, crouchSpeed * DeltaTime);
	crouchEyeOffSet = (1.0f - crouchTime) * crouchEyeOffSet;
}

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* enhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInput->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerBase::MovementPlayer);
		enhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerBase::LookPlayer);
		enhancedInput->BindAction(WeaponChangeAction, ETriggerEvent::Started, this, &APlayerBase::SelectWeapon);
		
		enhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerBase::JumpPlayer);
		enhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerBase::JumpPlayer);
		
		enhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerBase::Sprint);
		enhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerBase::Sprint);

		enhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerBase::CrouchPlayer);
		enhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerBase::CrouchPlayer);


	}



}



void APlayerBase::OnStartCrouch(float halfHeightAdjust, float scaledHalfHeightAdjust)
{
	if (halfHeightAdjust == 0.0f)
	{
		return;
	}

	float startBaseEyeHeight = BaseEyeHeight;
	Super::OnStartCrouch(halfHeightAdjust, scaledHalfHeightAdjust);
	crouchEyeOffSet.Z += startBaseEyeHeight - BaseEyeHeight + halfHeightAdjust;
	CameraFPS->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));

}

void APlayerBase::OnEndCrouch(float halfHeightAdjust, float scaledHalfHeightAdjust)
{
	if (halfHeightAdjust == 0.0f)
	{
		return;

	}

	float startBaseEyeHeight = BaseEyeHeight;
	Super::OnEndCrouch(halfHeightAdjust, scaledHalfHeightAdjust);
	crouchEyeOffSet.Z += startBaseEyeHeight - BaseEyeHeight - halfHeightAdjust;
	CameraFPS->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight), false);
}

void APlayerBase::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (CameraFPS)
	{
		CameraFPS->GetCameraView(DeltaTime, OutResult);
		OutResult.Location += crouchEyeOffSet;
	}

}



void APlayerBase::MovementPlayer(const FInputActionValue& valueInput)
{
	FVector2D movementVector = valueInput.Get<FVector2D>();

	//printf("%s", *movementVector.ToString());

	AddMovementInput(GetActorForwardVector(), movementVector.Y);
	AddMovementInput(GetActorRightVector(), movementVector.X);

}

void APlayerBase::LookPlayer(const FInputActionValue& valueInput)
{
	if (GetController() == nullptr)
	{
		return;
	}

	FVector2D axisVector = valueInput.Get<FVector2D>() * lookSensibility * GetWorld()->GetDeltaSeconds();

	AddControllerPitchInput(axisVector.Y);
	AddControllerYawInput(axisVector.X);
	
	
}

void APlayerBase::JumpPlayer(const FInputActionValue& valueInput)
{
	if (!isJumping)
	{
		Jump();
	}
	else
	{
		StopJumping();
	}
	isJumping = !isJumping;
}

void APlayerBase::Sprint(const FInputActionValue& valueInput)
{
	isSprint = !isSprint;

	GetCharacterMovement()->MaxWalkSpeed = (isSprint) ? sprintSpeed : normalSpeed;

}

void APlayerBase::CrouchPlayer(const FInputActionValue& valueInput)
{
	
	
	if (!isCrouched)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}

	isCrouched = !isCrouched;
}



void APlayerBase::SelectWeapon(const FInputActionValue& valueInput)
{

}


float APlayerBase::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
{
	currentHealth -= damageAmount;

	if (currentHealth <= 0)
	{
		OnDying();
	}

	printf("Current life = %f", currentHealth);

	return damageAmount;
}


void APlayerBase::OnDying_Implementation()
{
}

