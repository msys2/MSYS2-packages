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

    var cmdLocation = installer.value("TargetDir") + "\\msys2_shell.cmd";
    component.addOperation("CreateShortcut", cmdLocation, "@StartMenuDir@/MSYS2 MinGW 32-bit.lnk", "-mingw32");
    component.addOperation("CreateShortcut", cmdLocation, "@StartMenuDir@/MSYS2 MinGW 64-bit.lnk", "-mingw64");
    component.addOperation("CreateShortcut", cmdLocation, "@StartMenuDir@/MSYS2 MSYS.lnk", "-msys");

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
