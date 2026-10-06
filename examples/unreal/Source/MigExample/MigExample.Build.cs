using System.IO;
using UnrealBuildTool;

public class MigExample : ModuleRules
{
    public MigExample(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG" });
        PrivateDependencyModuleNames.Add("Projects");
        string sdk = Path.Combine(PluginDirectory, "ThirdParty");
        RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content", "raised-hands.json"),
            StagedFileType.NonUFS);
        PublicIncludePaths.Add(Path.Combine(sdk, "include"));
        if (target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(sdk, "lib", "mig-c.lib"));
            RuntimeDependencies.Add("$(TargetOutputDir)/mig-c.dll", Path.Combine(sdk, "bin", "mig-c.dll"));
        }
        else if (target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add(Path.Combine(sdk, "lib", "libmig-c.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libmig-c.so", Path.Combine(sdk, "lib", "libmig-c.so"));
        }
        else
        {
            throw new BuildException("MIG example supports desktop Windows/Linux only");
        }
    }
}
