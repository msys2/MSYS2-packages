function Component() {
    var systemDrive = installer.environmentVariable("SystemDrive");
    if (systemDrive === "") {
        systemDrive = "C:";
    }
    installer.setValue("ApplicationsDir", systemDrive+"\\msys@BITNESS@");
    installer.setDefaultPageVisible(QInstaller.Introduction, false);
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, true);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
}

Component.prototype.createOperationsForArchive = function(archive) {
    var ApplicationsDir = installer.value("ApplicationsDir");
    component.addElevatedOperation("Extract", archive, "@ApplicationsDir@");
}
