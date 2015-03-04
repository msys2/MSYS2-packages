function Component()
{
    // constructor
}

Component.prototype.isDefault = function()
{
    // select the component by default
    return true;
}

function createShortcuts()
{
    var windir = installer.environmentVariable("WINDIR");
    if (windir === "") {
        QMessageBox["warning"]( "Error" , "Error", "Could not find windows installation directory");
        return;
    }

    var cmdLocation = windir + "\\system32\\cmd.exe";
    component.addOperation( "CreateShortcut",
                            cmdLocation,
                            "@StartMenuDir@/Git for Windows SDK (MSYS2).lnk",
                            "/A /Q /K " + installer.value("TargetDir") + "\\msys2_shell.bat");

    component.addOperation( "CreateShortcut",
                            cmdLocation,
                            "@StartMenuDir@/Git for Windows SDK (MinGW 32-bit).lnk",
                            "/A /Q /K " + installer.value("TargetDir") + "\\mingw32_shell.bat");

    component.addOperation( "CreateShortcut",
                            cmdLocation,
                            "@StartMenuDir@/Git for Windows SDK (MinGW 64-bit).lnk",
                            "/A /Q /K " + installer.value("TargetDir") + "\\mingw64_shell.bat");

    if ("@BITNESS@bit" === "32bit") {
        component.addOperation( "Execute",
                               ["@TargetDir@\\autorebase.bat"]);
    }

    component.addOperation( "Execute",
                           ["@TargetDir@\\usr\\bin\\bash.exe", "--login", "-c", "exit"]);
}

Component.prototype.createOperations = function()
{
    component.createOperations();
    createShortcuts();
}
