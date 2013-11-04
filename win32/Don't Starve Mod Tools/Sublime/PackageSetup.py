#!python

from __future__ import with_statement
import glob
import os.path
import shutil
import zipfile


def isFileEntry(s):
	return s[-1] != '/'

def readFile(fname):
	with open(fname, 'rb') as f:
		return f.read()

def writeFile(fname, data):
	with open(fname, 'wb') as fo:
		fo.write(data)

def mkdirs(path):
	try:
		os.makedirs(path)
	except os.error:
		pass

def upgradeArchive(src, dst, pkgdir, backupdir, inhibitOverwrite):
	mkdirs(pkgdir)
	mkdirs(os.path.dirname(dst))

	try:
		newar = zipfile.ZipFile(src)
	except (zipfile.BadZipfile):
		print 'PackageSetup: bad zip file at %s' % src
		return
	newfiles = set(filter(isFileEntry, newar.namelist()))

	oldar = None
	oldfiles = set()
	try:
		oldar = zipfile.ZipFile(dst)
		oldfiles = set(filter(isFileEntry, oldar.namelist()))
	except (zipfile.error, IOError):
		pass

	# delete any orphaned files
	if not inhibitOverwrite:
		orphanedFiles = oldfiles - newfiles
		for f in orphanedFiles:
			# backup the file, if the user has modified it
			try:
				orig = oldar.read(f)
				user = readFile(os.path.join(pkgdir, f))
				if user != orig:
						backupFile = os.path.join(backupdir, f)
						mkdirs(os.path.dirname(backupFile))
						writeFile(backupFile, user)
			except (os.error, IOError):
				pass

			try:
				os.remove(os.path.join(pkgdir, f))
			except (os.error, IOError):
				pass

	# extract any new files
	for f in newfiles - oldfiles:
		try:
			if not isinstance(f, unicode):
				f = unicode(f, 'utf-8')
			fname = os.path.join(pkgdir, f)
		except (UnicodeDecodeError):
			fname = os.path.join(pkgdir, unicode(f, 'cp1252', 'replace'))

		mkdirs(os.path.dirname(fname))

		try:
			user = readFile(fname)
			if user:
				if inhibitOverwrite:
					continue;
				else:
					# Backup the old file
					backupFile = os.path.join(backupdir, f)
					mkdirs(os.path.dirname(backupFile))
					writeFile(backupFile, user)
		except (os.error, IOError):
			pass

		writeFile(fname, newar.read(f))

	# upgrade each file
	if not inhibitOverwrite:
		for f in oldfiles & newfiles:
			fname = os.path.join(pkgdir, f)
			orig = oldar.read(f)
			new = newar.read(f)

			if new != orig:
				# backup the file, if the user has modified it
				try:
					user = readFile(fname)
					if user != orig:
						backupFile = os.path.join(backupdir, f)
						mkdirs(os.path.dirname(backupFile))
						writeFile(backupFile, user)
				except (os.error, IOError):
					pass

				writeFile(fname, newar.read(f))

	# copy the zip
	shutil.copy(src, dst)

def removeArchive(dst, pkgdir):
	oldar = None
	oldfiles = set()
	try:
		oldar = zipfile.ZipFile(dst)
		oldfiles = set(filter(isFileEntry, oldar.namelist()))
		oldar.close()
	except (zipfile.error, IOError):
		pass

	# delete any orphaned files
	orphanedFiles = oldfiles
	for f in orphanedFiles:
		try:
			os.remove(os.path.join(pkgdir, f))
		except (os.error, IOError):
			pass

	# delete the archive and destination
	try:
		os.remove(dst)
	except (os.error, IOError):
		pass

	try:
		os.rmdir(pkgdir)
	except (os.error, IOError):
		pass

def srcNewer(src, dst):
	try:
		return (os.path.getmtime(src) > os.path.getmtime(dst)
			or os.path.getsize(src) != os.path.getsize(dst))
	except (os.error, IOError):
		return True

def upgradePackage(pkg, pristinedir, datadir, backupdir):
	pristinePkg = os.path.join(pristinedir, os.path.basename(pkg))
	# if the zip is different from the one in pristinedir
	if srcNewer(pkg, pristinePkg):
		(base, ext) = os.path.splitext(os.path.basename(pkg))
		inhibitOverwrite = (base == "User")
		upgradeArchive(pkg, pristinePkg, os.path.join(datadir, base),
			os.path.join(backupdir, base), inhibitOverwrite)

def upgrade(appdir, userdir, pristinedir, datadir, backupdir):
	packages = (glob.glob(appdir + "/*.sublime-package") +
			glob.glob(userdir + "/*.sublime-package"))

	for pkg in packages:
		upgradePackage(pkg, pristinedir, datadir, backupdir)

	# Delete any packages that are no longer around
	depzips = (set([os.path.basename(x) for x in glob.glob(pristinedir + "/*.sublime-package")])
		- set([os.path.basename(x) for x in packages]))
	for dz in depzips:
		pz = os.path.join(pristinedir, dz)
		(base, ext) = os.path.splitext(dz)
		removeArchive(pz, os.path.join(datadir, base))
