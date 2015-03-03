function Component() {
    var systemDrive = installer.environmentVariable("SystemDrive");
    // Use C: as a default for messed up systems.
    if (systemDrive === "") {
        systemDrive = "C:";
    }
    installer.setValue("TargetDir", systemDrive+"\\git-sdk-@BITNESS@bit");
    installer.setDefaultPageVisible(QInstaller.Introduction, false);
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, true);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
}
