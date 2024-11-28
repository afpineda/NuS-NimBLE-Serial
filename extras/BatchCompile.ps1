<############################################################################

.SYNOPSYS
    Compile all sketches in batch

.AUTHOR
    Ángel Fernández Pineda. Madrid. Spain. 2024.

#############################################################################>

#setup
$ErrorActionPreference = 'Stop'

$thisPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
Set-Location "$thispath/.."

# global constants
$_compiler = "arduino-cli"

<#############################################################################
# Auxiliary functions
#############################################################################>
function Test-ArduinoCLI {
    &$_compiler version | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "Arduino-cli not found. Check your PATH."
    }
}

function Invoke-ArduinoCLI {
    param(
        [string]$Filename,
        [string]$BuildPath
    )
    Write-Host "--------------------------------------------------------------------------------"
    Write-Host "Sketch: $Filename"
    Write-Host "================================================================================"
    $ErrorActionPreference = 'Continue'
    &$_compiler compile $Filename -b esp32:esp32:esp32 --no-color --warnings all --build-path $BuildPath # 2>&1
    $ErrorActionPreference = 'Stop'
}

function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force | Out-Null
    $tempFolderName = Join-Path $ENV:Temp $File.Name
    $Folder = New-Item -Itemtype Directory -Path $tempFolderName
    return $Folder
}

<#############################################################################
# MAIN
#############################################################################>

# Initialization
$VerbosePreference = "continue"
$InformationPreference = "continue"
Test-ArduinoCLI

# Create a temporary folder (will speed up compilation)
$tempFolder = New-TemporaryFolder

try {
    Clear-Host
    Write-Host "************"
    Write-Host "* EXAMPLES *"
    Write-Host "************"
    Invoke-ArduinoCLI -Filename "examples/ATCommandDemo/ATCommandDemo.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "examples/ATCommandDemoLegacy2/ATCommandDemoLegacy2.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "examples/CustomCommandProcessor/CustomCommandProcessor.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "examples/NuSEcho/NuSEcho.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "examples/NuSerialDump/NuSerialDump.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "examples/ShellCommandDemo/ShellCommandDemo.ino" -BuildPath $tempFolder
    Write-Host "*********"
    Write-Host "* TESTS *"
    Write-Host "*********"
    Invoke-ArduinoCLI -Filename "extras/test/ATCommandsTester/ATCommandsTester.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "extras/test/ATCommandsTesterLegacy2/ATCommandsTesterLegacy2.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "extras/test/HandshakeTest/HandshakeTest.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "extras/test/Issue8/Issue8.ino" -BuildPath $tempFolder
    Invoke-ArduinoCLI -Filename "extras/test/SimpleCommandTester/SimpleCommandTester.ino" -BuildPath $tempFolder
}
finally {
    # Remove temporary folder
    Remove-Item -Recurse -LiteralPath $tempFolder.FullName -Force | Out-Null
}