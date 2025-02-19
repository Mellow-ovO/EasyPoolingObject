using UnrealBuildTool;

public class EasyPoolingObjectDev : ModuleRules
{
    public EasyPoolingObjectDev(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "EasyPoolingObject",
                "Kismet",
                "KismetCompiler",
                "UnrealEd",
                "BlueprintGraph"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}