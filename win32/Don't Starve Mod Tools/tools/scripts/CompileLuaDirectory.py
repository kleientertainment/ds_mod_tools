#* This script compiles all .lua files in the current directory, recursively.
#* It will also backup scripts into a folder labeled 'src-backup' if desired.
#* 
#* @author: John_Michael <JohnMichaelFFS@gmail.com>
#* @version: 2/2/2012
import os
import shutil

#* This function compiles all files with a .lua file extension in the current directory, via luac.
#* This function will also attempt to backup the scripts, if requested.
#*
#* @return True if successfull, False otherwise.
def compileLuaDirectory(resourceDirectory=os.getcwd()):
	# Backup the resource directory, if requested.
	userInput = raw_input("* Would you like to back up %s? All current backups will be lost! [y/n]\n"%(resourceDirectory))
	if userInput == "y":
		backupSuccessfull = backupDirectory(resourceDirectory)
		if not backupSuccessfull:
			return False
	
	# Iterate through all files in the current directory recursively, searching for .lua files
	print("* Compiling scripts in %s..."%(resourceDirectory))
	for currentDirectory, subDirectories, filePaths in os.walk(resourceDirectory):
		for filePath in filePaths:
			if ".lua" in filePath and not "backup" in currentDirectory:
				scriptPath = os.path.join(currentDirectory, filePath)
				print("* Compiling script %s..."%(scriptPath))

				# Compile the script
				try:
					os.system("luac -o %s %s"%(scriptPath, scriptPath))
				except Exception as errorMessage:
					print("ERROR: Failed to compile script %s! Reason: %s\n"%(scriptPath, errorMessage))
					return False

	print("* All Lua scripts in %s have been compiled successfully!\n"%(resourceDirectory))
	return True

#* This function backups all files in the current directory to the src-backup folder.
#*
#* @return True if successfull, False otherwise.
def backupDirectory(directoryToBackup):
	backupPath = os.path.join(directoryToBackup, "backup")
	print("* Backing up %s to %s..."%(directoryToBackup, backupPath))
	if os.path.exists(backupPath):
		shutil.rmtree(backupPath)

	try:
		shutil.copytree(directoryToBackup, backupPath)
	except:
		print("ERROR: Directory backup failed! Please ensure the backup folder does not already exist.\n")
		return False

	print("* Backup complete!\n")
	return True

def main():
	print("== Lua Directory Compiler by John_Michael <JohnMichaelFFS@gmail.com> ==\n")
	compileLuaDirectory()
	os.system('pause')

main()
