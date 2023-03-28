// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"


AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 0.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	holdingShift = false;
}


void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 360.0f, FColor::Black, TEXT("Click + Hold Shift: Move object"));
		GEngine->AddOnScreenDebugMessage(2, 360.0f, FColor::Black, TEXT("Click + Hold shift + Mouse Wheel: Resize object"));
	}
	
}



void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (target)
	{
		if (holdingShift)
		{
			MoveObject();
		}
	}
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//player movements
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AMyCharacter::LookUp);

	//object interactions
	InputComponent->BindAction(FName("Cast"), IE_Pressed, this, &AMyCharacter::Ray);
	InputComponent->BindAction(FName("MoveObject"), IE_Pressed, this, &AMyCharacter::ShiftOnOff);
	InputComponent->BindAction(FName("MoveObject"), IE_Released, this, &AMyCharacter::ShiftOnOff);
	InputComponent->BindAxis(FName("UniformResize"), this, &AMyCharacter::UniformResize);
}

void AMyCharacter::MoveForward(float Value)
{
	if (Controller && (Value !=0.f))
	{
		
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);//which way is forward

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Controller && (Value != 0.f))
	{
		
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);//which way is right

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::Turn(float Value)
{
	if (Controller && (Value != 0.f))
	{
		AddControllerYawInput(Value);
	}
}

void AMyCharacter::LookUp(float Value)
{
	if (Controller && (Value != 0.f))
	{
		AddControllerPitchInput(Value);
	}
}

void AMyCharacter::Ray()
{
	FVector start = ViewCamera->GetComponentLocation();
	FVector forward = ViewCamera->GetForwardVector(); 
	start = FVector(start.X + (forward.X * 100), start.Y + (forward.Y * 100), start.Z + (forward.Z * 100)); //so the ray starts a little in front of the camera
	FVector end = start + (forward * 1000); //from how far we should be able to grab objects;
	FHitResult hit;

	if (GetWorld())
	{
		bool actorHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Pawn, FCollisionQueryParams(), FCollisionResponseParams());
		
		if (actorHit && hit.GetActor()) //if raycast is successful
		{
			if (holdingShift)
			{
				target = hit.GetActor();
				mesh = target->FindComponentByClass<UStaticMeshComponent>();
				mesh->SetSimulatePhysics(false); //move the object without it falling down constantly
				diff = target->GetActorLocation() - hit.ImpactPoint;
			}
			
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, hit.GetActor()->GetFName().ToString());
		}
	}

}

void AMyCharacter::ShiftOnOff()
{
	holdingShift = !holdingShift;
	if (!holdingShift)
	{
		if (mesh)
		{
			mesh->SetSimulatePhysics(true); //let the object fall back down
		}
		mesh = NULL;
		target = NULL;
	}
}

void AMyCharacter::MoveObject()
{
	FVector forward = ViewCamera->GetForwardVector();
	FVector currentLoc = target->GetActorLocation();
	FVector loc = ViewCamera->GetComponentLocation();
	loc += diff;

	float dist = FVector::Distance(loc, currentLoc);

	target->SetActorRelativeLocation(FVector(loc.X + (forward.X * dist), loc.Y + (forward.Y * dist), loc.Z + (forward.Z * dist)));
}

void AMyCharacter::UniformResize(float Value)
{
	if (Value)
	{
		if (target && holdingShift)
		{
			FVector scale = target->GetActorScale3D();
			if (scale.Size() < 10 && Value > 0) //grow object
			{
				target->SetActorScale3D(scale * 1.05f);
			}
			if (scale.Size() > 0.5f && Value < 0) //shrink object
			{
				target->SetActorScale3D(scale * 0.95f);
			}
		}
	}
}
