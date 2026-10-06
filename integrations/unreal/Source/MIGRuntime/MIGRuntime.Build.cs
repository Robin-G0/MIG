using System.IO;
using UnrealBuildTool;

public class MIGRuntime : ModuleRules
{
    public MIGRuntime(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        string sdk = Path.Combine(PluginDirectory, "ThirdParty");
        PublicIncludePaths.Add(Path.Combine(sdk, "include"));
        if (target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(sdk, "lib", "mig-c.lib"));
            RuntimeDependencies.Add("$(TargetOutputDir)/mig-c.dll", Path.Combine(sdk, "bin", "mig-c.dll"));
        }
        else if (target.Platform == UnrealTargetPlatform.Linux ||
                 target.Platform == UnrealTargetPlatform.LinuxArm64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(sdk, "lib", "libmig-c.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libmig-c.so.1", Path.Combine(sdk, "lib", "libmig-c.so.1"));
        }
        else
        {
            throw new BuildException("MIG requires a packaged Windows or Linux C ABI SDK");
        }
    }
}
