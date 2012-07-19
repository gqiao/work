#!/usr/bin/perl -w
#
# Copyright 1998 VMware, Inc.  All rights reserved. -- VMware Confidential
#
# VMware Virtual Hard Disk mount script

use strict;

# External helper programs
#  mknod binary location
my $mknod;
#  modprobe binary location
my $modprobe;
#  mount binary location
my $mount;
#  umount binary location
my $umount;
#  uname binary location
my $uname;
#  rm binary location
my $rm;

# Prefix for temporary directories
my $cTmpDirPrefix = 'vmware-mount';

# Path of vmware-loop
my $vmware_loop;

# nbd prefix
my $nbd_prefix = "/dev/nb";

# Global variables for signal handlers
#  PID of the child process
my $child_pid;
#  Mounted device
my $mount_dev;
#  Mounted directory
my $mount_pt;

# Size of a terminal line
my $cTerminalLineSize = 80;

# Convert a string to its equivalent shell representation
sub shell_string {
  my $single_quoted = $_[0];

  $single_quoted =~ s/'/'"'"'/g;
  # This comment is a fix for emacs's broken syntax-highlighting code --hpreg
  return '\'' . $single_quoted . '\'';
}

# Execute the command passed as an argument
# _without_ interpolating variables (Perl does it by default)
sub direct_command {
  return `$_[0]`;
}

# Wordwrap system: append some content to the output
sub append_output {
  my $output = shift;
  my $pos = shift;
  my $append = shift;

  $output .= $append;
  $pos += length($append);
  if ($pos >= $cTerminalLineSize) {
    $output .= "\n";
    $pos = 0;
  }

  return ($output, $pos);
}

# Wordwrap system: deal with the next character
sub wrap_one_char {
  my $output = shift;
  my $pos = shift;
  my $word = shift;
  my $char = shift;
  my $reserved = shift;
  my $length;

  if (not (($char eq "\n") || ($char eq ' ') || ($char eq ''))) {
    $word .= $char;

    return ($output, $pos, $word);
  }

  # We found a separator. Process the last word

  $length = length($word) + $reserved;
  if (($pos + $length) > $cTerminalLineSize) {
    # The last word doesn't fit in the end of the line. Break the line before it
    $output .= "\n";
    $pos = 0;
  }
  ($output, $pos) = append_output($output, $pos, $word);
  $word = '';

  if ($char eq "\n") {
    $output .= "\n";
    $pos = 0;
  } elsif ($char eq ' ') {
    if ($pos) {
      ($output, $pos) = append_output($output, $pos, ' ');
    }
  }

  return ($output, $pos, $word);
}

# Wordwrap system: word-wrap a string plus some reserved trailing space
sub wrap {
  my $input = shift;
  my $reserved = 0;
  my $output;
  my $pos;
  my $word;
  my $i;

  $output = '';
  $pos = 0;
  $word = '';
  for ($i = 0; $i < length($input); $i++) {
    ($output, $pos, $word) = wrap_one_char($output, $pos, $word, substr($input, $i, 1), 0);
  }
  # Use an artifical last '' separator to process the last word
  ($output, $pos, $word) = wrap_one_char($output, $pos, $word, '', $reserved);

  return $output;
}

# Build a digital Linux kernel version number
sub Kernel_MakeVersion {
  my $version;
  my $patchLevel;
  my $subLevel;
  ($version, $patchLevel, $subLevel) = @_;

  return $version * 65536 + $patchLevel * 256 + $subLevel;
}

# Get a clean version number for the running Linux kernel
# Return value is the list of:
#  Complete human readable version
#  Clean x.y.z human readable version
#  Digital version (default result)
sub Kernel_RunningVersion {
  my $fullVersion;
  my $version;
  my $patchLevel;
  my $subLevel;

  $fullVersion = direct_command(shell_string($uname) . ' -r');
  chop($fullVersion);
  ($version, $patchLevel, $subLevel) = split(/\./, $fullVersion);
  # Clean the subLevel in case there is an extraversion
  ($subLevel) = split(/[^0-9]/, $subLevel);

  return ($fullVersion, $version . '.' . $patchLevel . '.' . $subLevel, Kernel_MakeVersion($version, $patchLevel, $subLevel));
}

# Contrary to a popular belief, 'which' is not always a shell builtin command.
# So we can not trust it to determine the location of other binaries.
# Moreover, SuSE 6.1's 'which' is unable to handle program names beginning with
# a '/'...
#
# Return value is the complete path if found, or "" if not found
sub internal_which {
  my $bin = shift;

  if (substr($bin, 0, 1) eq '/') {
    # Absolute name
    if ((-f $bin) && (-x $bin)) {
      return $bin;
    }
  } else {
    # Relative name
    my @paths;
    my $path;

    if (index($bin, '/') == -1) {
      # There is no other '/' in the name
      @paths = split(':', $ENV{'PATH'});
      foreach $path (@paths) {
	my $fullbin;

	$fullbin = $path . '/' . $bin;
	if ((-f $fullbin) && (-x $fullbin)) {
	  return $fullbin;
	}
      }
    }
  }

  return "";
}

# Tell if the user is the super user
sub is_root {
  return $> == 0;
}

# Prompt the user for a value, providing a default value
sub query {
  my ($message, $defaultreply) = @_;
  my $reply;
  my $optSpace = substr($message, -1) eq "\n" ? "" : " ";

  print "\n";
  if ($defaultreply ne "") {
    print wrap($message . $optSpace . "[" . $defaultreply . "] ");
  } else {
    print wrap($message . $optSpace);
  }
  
  chop($reply = <STDIN>);
  $reply =~ s/^\s*//;			# Kill leading whitespace
  $reply =~ s/\s*$//;			# Kill trailing whitespace
  $reply = $defaultreply if $reply eq "";
  return $reply;
}

# Prompts the user if a binary is not found
# Return value is:
#  "": the binary has not been found
#  the binary name if it has been found
sub DoesBinaryExist_Prompt {
  my $bin;
  my $ans;

  $bin = $_[0];
  while (internal_which($bin) eq "") {
    $ans = query("Setup is unable to find the " . $bin . " program on your machine. Do you want to specify the location of this program by hand?", "Y");
    if ($ans !~ /^[yY]/) {
      return "";
    }

    $bin = query("What is the location of the " . $bin . " program on your machine?", "");
  }
  return $bin;
}

# BEGINNING_OF_TMPDIR_DOT_PL
#!/usr/bin/perl

use strict;

# Create a temporary directory
#
# They are a lot of small utility programs to create temporary files in a
# secure way, but none of them is standard. So I wrote this --hpreg
sub make_tmp_dir {
  my $prefix = shift;
  my $tmp;
  my $serial;
  my $loop;

  $tmp = defined($ENV{'TMPDIR'}) ? $ENV{'TMPDIR'} : '/tmp';

  # Don't overwrite existing user data
  # -> Create a directory with a name that didn't exist before
  #
  # This may never succeed (if we are racing with a malicious process), but at
  # least it is secure
  $serial = 0;
  for (;;) {
    # Check the validity of the temporary directory. We do this in the loop
    # because it can change over time
    if (not (-d $tmp)) {
      error('"' . $tmp . '" is not a directory.' . "\n\n");
    }
    if (not ((-w $tmp) && (-x $tmp))) {
      error('"' . $tmp . '" should be writable and executable.' . "\n\n");
    }

    # Be secure
    # -> Don't give write access to other users (so that they can not use this
    # directory to launch a symlink attack)
    if (mkdir($tmp . '/' . $prefix . $serial, 0755)) {
      last;
    }

    $serial++;
    if ($serial % 200 == 0) {
      print STDERR 'Warning: The "' . $tmp . '" directory may be under attack.' . "\n\n";
    }
  }

  return $tmp . '/' . $prefix . $serial;
}

# END_OF_TMPDIR_DOT_PL

# Prompts the user if a file is not readable
# Return value is:
#  "": the file is not readable
#  the file path if it has been found
sub IsFileReadable_Prompt {
  my $bin;
  my $ans;

  $bin = $_[0];
  while (not -r $bin) {
    $ans = query("Setup is unable to read the " . $bin . " file. Do you want to specify the location of this file by hand?", "Y");
    if ($ans !~ /^[yY]/) {
      return "";
    }

    $bin = query("What is the location of the " . $bin . " file on your machine?", "");
  }
  return $bin;
}

# Set up the location of external helpers
sub initialize_external_helpers {
  $mknod = DoesBinaryExist_Prompt("mknod");
  if ($mknod eq "") {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }
  $modprobe = DoesBinaryExist_Prompt("modprobe");
  if ($modprobe eq "") {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }
  $mount = DoesBinaryExist_Prompt("mount");
  if ($mount eq "") {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }
  $umount = DoesBinaryExist_Prompt("umount");
  if ($umount eq "") {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }
  $uname = DoesBinaryExist_Prompt("uname");
  if ($uname eq "") {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }
  $rm = DoesBinaryExist_Prompt('rm');
  if ($rm eq '') {
    print STDERR wrap("\n" . 'Unable to continue.' . "\n");
    exit(1);
  }
}

# Retrieve the path to the vmware-loop binary
sub get_loop_binary {
  my $vmware_config;
  my $line;
  my @parts;

  $vmware_config = IsFileReadable_Prompt("/etc/vmware/config");
  if ($vmware_config eq "") {
    print STDERR wrap("\nUnable to find the system wide VMware installation file. You may want to re-install VMware on your computer.\n");
    exit(1);
  }
  $vmware_loop = "";
  open(VMCONF, $vmware_config);
  while (defined($line = <VMCONF>)) {
    chop $line;
    if ($line =~ /^[ \t]*loop\.fullpath[ \t]*=[ \t]*/) {
      @parts = split(/\"/, $line);
      $vmware_loop = $parts[1];
    }
  }
  close(VMCONF);

  if ($vmware_loop eq "") {
    print STDERR wrap("\nThis script can not find a correctly formatted loop.fullpath entry in the " . $vmware_config . " file. You may want to re-install VMware on your computer.\n");
    exit(1);
  }
}

# Display the splash screen
sub splash_screen {
  print "\n"
. "--------------------------------------------\n"
. "VMware for Linux - Virtual Hard Disk Mounter\n"
. 'Version: 1.0 build-91891'
. "\nCopyright 1998 VMware, Inc.  All rights reserved. -- VMware Confidential\n"
. "--------------------------------------------\n";
}

# Find if a network block device exists and is a block device
sub does_nbd_exist {
  my $nr = $_[0];

  return -b $nbd_prefix . $nr
}

# Create a network block device
sub create_nbd {
  my $nr = $_[0];
  my $ans;
  my $begin;

  if ($nr == 0) {
    $begin = "There is no Network Block Device defined on this machine.";
  } else {
    $begin = "All Network Block Devices are currently in use.";
  }
  $ans = query($begin . " This script is about to create the " . $nbd_prefix . $nr . " Network Block Device. Continue?", "Y");
  if ($ans !~ /^[yY]/) {
    print STDERR wrap("\nUnable to continue.\n");
    exit(1);
  }

  print wrap("\nCreating the " . $nbd_prefix . $nr . " Network Block Device\n");
  if (system(shell_string($mknod) . " " . $nbd_prefix . $nr . " b 43 " . $nr . " > /dev/null")) {
    print STDERR wrap("\nUnable to create the " . $nbd_prefix . $nr . " Network Block Device. You may need to re-run this script as the super user.\n");
    exit(1);
  }

  if (not chmod 0600, $nbd_prefix . $nr) {
    print STDERR wrap("\nUnable to change the permission of the " . $nbd_prefix . $nr . " file.\n");
    exit(1);
  }
}

# Find if a network block device driver exists
sub does_nbd_driver_exist {
  my $nr = $_[0];

  if (open DEV, $nbd_prefix . $nr) {
    print wrap("\nNetwork Block Device driver detected.\n");
    close(DEV);
    return 1;
  }

  print wrap("\nNo Network Block Device driver detected.\n");
  return 0;
}

# Kill the child
sub forward_kill {
  kill 'USR1', $child_pid;
}

# Determine if the mount point is mounted as it should
sub IsMounted {
  my $mount_pid;
  my $line;
  my @parts;

  $mount_pid = open MOUNT, shell_string($mount) . " |";
  if (not defined($mount_pid)) {
    print STDERR wrap("\nUnable to invoke " . $mount . ".\n");
    exit(1);
  }

  while (defined($line = <MOUNT>)) {
    chop($line);
    @parts = split(/[ \t]+/, $line);
    # Use the device name for the comparison because it can not contain
    # a " " or a trailing "/" and cannot be relative
    if ($parts[0] eq ($nbd_prefix . $mount_dev)) {
      return 1;
    }
  }
  close(MOUNT); 

  return 0;
}

# SIGINT handler
sub sigint_handler {
  my $signame = shift;

  if (IsMounted()) {
    # Unmount the nbd before killing the child
    if (system(shell_string($umount) . " " . $nbd_prefix . $mount_dev . " > /dev/null")) {
      print wrap("\nUnable to unmount the Network Block Device on the " . $mount_pt . " mount point. Please make sure that no process uses " . $mount_pt . " as its current directory or is accessing a file under this directory.\n\nUsing another terminal, you can continue to browse your Virtual Hard Disk partition in " . $mount_pt . ". Hit Control-C again in this terminal when done.\n");
      return;
    }
  } else {
    print STDERR wrap("\nIt seems like somebody unmounted the " . $mount_pt . " mount point in my back. I was about to do it anyway.\n");
  }

  forward_kill();
}

# Retrieve the filesystem of the partition from the partition type and Id
sub get_partition_fs {
  my $tmpfile = $_[0];
  my $part_nb = $_[1];
  my $line;
  my @parts;
  my $offset;
  my $part_type;
  my $part_id;

  if (not open(OUTPUT, $tmpfile)) {
    printf STDERR wrap("\nUnable to open the temporary file containing the partition table.\n");
    return "";
  }

  $part_type = "";
  $part_id = "";
  $line = <OUTPUT>;
  $line = <OUTPUT>;
  $line = <OUTPUT>;
  while(defined($line = <OUTPUT>)) {
    chop $line;
    @parts = split(/[ \t]+/, $line);
    # In case the line begins with a whitespace
    if ($parts[0] eq "") {
      $offset = 1;
    } else {
      $offset = 0;
    }
    if ($part_nb == $parts[$offset]) {
      $part_type = $parts[$offset + 3];
      $part_id = $parts[$offset + 4];
      last;
    }
  }

  close(OUTPUT);
  if (($part_type eq "") || ($part_id eq "")) {
    printf STDERR wrap("\nUnable to retrieve the partition in the partition table.\n");
    return "";
  }

  if ($part_type eq "BIOS") {
    if (($part_id eq "1") || ($part_id eq "4") || ($part_id eq "6") || ($part_id eq "B") || ($part_id eq "E") || ($part_id eq "11") || ($part_id eq "14") || ($part_id eq "16") || ($part_id eq "24") || ($part_id eq "C1") || ($part_id eq "C4") || ($part_id eq "C6") || ($part_id eq "F2")) {
      return "msdos";
    }
    if ($part_id eq "C") {
      return "vfat";
    }
    if ($part_id eq "7" || ($part_id eq "17")) {
      return "ntfs";
    }
    if ($part_id eq "4D" || ($part_id eq "4E") || ($part_id eq "4F")) {
      return "qnx4";
    }
    if ($part_id eq "83") {
      return "ext2";
    }
  }
  if ($part_type eq "BSD") {
    if ($part_id eq "7") {
      return "ufs";
    }
    if ($part_id eq "11") {
      return "ext2";
    }
  }

  print STDERR wrap("\nUnable to retrieve the filesystem of the partition (the partition type is " . $part_type . " and the partition Id is " . $part_id . "). Please file an incident with VMware at http://www.vmware.com/forms/Incident_Login.cfm by copying this error message.\n");
  return "";
}

# Remove a temporary directory
sub remove_tmp_dir {
  my $dir = shift;

  if (system(shell_string($rm) . ' -rf ' . shell_string($dir))) {
    print STDERR wrap("\n" . 'Unable to remove the temporary directory ' . $dir . '.' . "\n");
  };
}

# Export and mount a partition
sub export_mount {
  my $vhd = $_[0];
  my $part_nb = $_[1];
  my $vfstype = $_[2];
  # $_[3] is the global $mount_pt
  my $options = $_[4];
  my $i;
  my $fuser;
  my $line;
  my $char;
  my $ready;
  my $retval;
  my $tmpdir;
  my $tmpfile;

  # Find an existing nbd
  for(;;) {
    for($i = 0; $i < 256; $i++) {
      if (does_nbd_exist($i)) {
	last;
      }
    }
    
    if ($i != 256) {
      last;
    }
    print wrap("\nNo Network Block Device detected.\n");
    create_nbd(0);
  }

  # Make sure the nbds are supported
  if (not does_nbd_driver_exist($i)) {
    print wrap("\nTrying to load the Network Block Device driver kernel module... ");
    if (system(shell_string($modprobe) . " nbd > /dev/null")) {
      print "Failure.\n";
      print STDERR wrap("\nIf you are not an experimented Linux user, you may want to upgrade your Linux distribution and re-run this script.\n\nIf you are an experimented Linux user, you may want to re-compile your kernel. Use a kernel version 2.1.101 or higher, and set the CONFIG_BLK_DEV_NBD option (which you can find under \"Block devices\"->\"Network block device support\") to \"y\" or \"m\". If you don't want to mess with kmod (the kernel module loader formerly known as kerneld), we recommend to use \"y\". Once you have rebooted your new kernel, re-run this script.\n");
      exit(1);
    }

    print "Success.\n";
  }

  # Output the partition table in a temporary file (so that it doesn't
  # interfere with user interaction). Do that before mapping the
  # virtual hard disk partition to an nbd to avoid the deadlock
  $tmpdir = make_tmp_dir($cTmpDirPrefix);
  $tmpfile = $tmpdir . '/partition_table';
  if (system(shell_string($vmware_loop) . " -q -P " . shell_string($tmpfile) . " " . shell_string($vhd))) {
    print STDERR wrap("\nUnable to invoke " . $vmware_loop . ". You may want to re-install VMware.\n");
    exit(1);
  }

  # As of Linux 2.2.9, the nbd driver allows to issue 2 NBD_SET_SOCK ioctls
  # in a row without NBD_CLEAR_SOCK in between. This is not cool because we
  # cannot know if another process has already set up the nbd :( I'm going to
  # submit a patch. In the meantime, I have no other option than adding a race
  # condition by using fuser or lsof :( --hpreg
  $fuser = DoesBinaryExist_Prompt("fuser");
  if ($fuser eq "") {
    $fuser = DoesBinaryExist_Prompt("lsof");
    if ($fuser eq "") {
      print STDERR wrap("\nThis script can not continue.\n");
      remove_tmp_dir($tmpdir);
      exit(1);
    }
  }
  
  # Map the virtual hard disk partition to the first available nbd
  for($i = 0; $i < 256; $i++) {
    if (not does_nbd_exist($i)) {
      create_nbd($i);
    }

    if (not system(shell_string($fuser) . " " . $nbd_prefix . $i . " > /dev/null")) {
      print wrap("The " . $nbd_prefix . $i . " Network Block Device is busy.\n");
      next;
    }

    select(STDOUT); $| = 1; # Unbuffer
    # Fork a vmware-loop in quiet_mode, and redirect its stdin/stdout to the
    # LOOP filehandle
    $child_pid = open LOOP, shell_string($vmware_loop) . " -q " . shell_string($vhd) . " " . $part_nb . " " . $nbd_prefix . $i . " |";
    if (not defined($child_pid)) {
      print STDERR wrap("\nUnable to invoke " . $vmware_loop . ". You may want to re-install VMware.\n");
      remove_tmp_dir($tmpdir);
      exit(1);
    }

    $line = "";
    for (;;) {
      $char = getc(LOOP);
      if (not defined ($char)) {
	# EOF
	$ready = 0;
        last;
      }

      # Display the output of the program so that the user can interact with
      # the hints
      print $char;

      # Reconstitute lines
      if ($char eq "\n") {
	if ($line =~ /^Client: The partition is now mapped on the $nbd_prefix$i Network Block Device.$/) {
	  $ready = 1;
	  last;
	}
	$line = "";
      } else {
	$line = $line . $char;
      }
    }

    if ($ready == 0) {
      # Child closed the pipe, its exit code is in $?
      # This shouldn't normally happen unless the binary location is wrong
      print wrap("Unable to map the " . $nbd_prefix . $i . " Network Block Device.\n");
    } else {
      last;
    }
  }
  if ($i == 256) {
    print STDERR wrap("\nAll Network Block Devices are in use. Please re-run this script when at least one of them is not busy anymore.\n");
    remove_tmp_dir($tmpdir);
    exit(1);
  }

  # Avoid to special case this later
  if (not ($options eq "")) {
    $options = '-o ' . shell_string($options);
  }

  # Mount the nbd
  $mount_pt = $_[3];
  $mount_dev = $i;
  if ($vfstype eq "") {
    # First try to autoprobe the type of the filesystem (in case its driver is
    # either non-modular, or modular and already loaded)
    $retval = system(shell_string($mount) . ' ' . $options . ' ' . $nbd_prefix . $mount_dev . ' ' . shell_string($mount_pt) . ' > /dev/null');
    if ($retval) {
      # Second, try to deduce the type from the partition table
      print wrap("\nIf you know the filesystem of the partition you want to mount, you can provide it using the -t command line option. Since you haven't done so, this script is going to try to determine the filesystem of the partition based on the partition type and id.\n");

      $vfstype = get_partition_fs($tmpfile, $part_nb);
      if ($vfstype eq "") {
	forward_kill();
	close(LOOP);
        remove_tmp_dir($tmpdir);
	exit(1);
      }      
      
      print wrap("\nDetected " . $vfstype . " filesystem type.\n");
      $retval = system(shell_string($mount) . ' ' . $options . ' -t ' . shell_string($vfstype) . ' ' . $nbd_prefix . $mount_dev . ' ' . shell_string($mount_pt) . ' > /dev/null');
    }
  } else {
    # Use the type provided by the user
    $retval = system(shell_string($mount) . ' ' . $options . ' -t ' . shell_string($vfstype) . ' ' . $nbd_prefix . $mount_dev . ' ' . shell_string($mount_pt) . ' > /dev/null');
  }
  if ($retval) {
    print wrap("\nUnable to mount the Network Block Device on the " . $mount_pt . " mount point. Please make sure that nothing is currently using the mount point and that your kernel supports the partition type you want to mount before re-running this script.\n");
    forward_kill();
    close(LOOP);
    remove_tmp_dir($tmpdir);
    exit(1);
  }

  remove_tmp_dir($tmpdir);
  
  # Install the SIGINT handler
  $SIG{INT} = \&sigint_handler;

  print wrap("\nUsing another terminal, you can now browse your Virtual Hard Disk partition in " . $mount_pt . ". Hit Control-C in this terminal when done.\n");

  for (;;) {
    $char = getc(LOOP);
    if (not defined ($char)) {
      # EOF
      last;
    }

    # Display the output of the program so that the user can interact with
    # the hints
    print $char;
  }

  # Child has exited (perhaps because we killed it)
  close(LOOP);

  # Reset the handler
  $SIG{INT} = 'DEFAULT';
}

# Display the partition table of a virtual disk
sub show_partition_table {
  my $vhd = $_[0];

  if (system(shell_string($vmware_loop) . " -q -p " . shell_string($vhd))) {
    print STDERR wrap("\nUnable to invoke " . $vmware_loop . ". You may want to re-install VMware.\n");
    exit(1);
  }
}

# Display a usage string on stderr, and exit
sub usage {
  print STDERR "\nUsage: " . $0 . "\n" .
"\t-p          : Print the partition table\n" .
"\tdisk        : Name of the Virtual Hard Disk file\n" .
"or\n" .
"\tdisk        : Name of the Virtual Hard Disk file\n" .
"\tpartition   : Number of the partition\n" .
"\t[-t type]   : Partition type\n" .
"\t[-o options]: Partition mount options(s)\n" .
"\tmount-point : Directory where to mount the partition\n";
  exit(1);
}

# Where the real work is done
sub main {  
  #  print partition table
  my $print_mode;
  #  filesystem type
  my $type_value;
  #  mount options
  my $options_value;
  #  mandatory arguments
  my @arguments;

  my $i;
  my $nb;

  # Scan the command line
  @arguments = ();
  $print_mode = 0;
  $type_value = "";
  $options_value = "";
  for ($i = 0; $i < $#ARGV + 1; $i++) {
    if ($ARGV[$i] eq "--") {
      last;
    }
    if ($ARGV[$i] eq "-p") {
      $print_mode = 1;
      next;
    }
    if ($ARGV[$i] eq "-t") {
      $i++;
      if ($i >= $#ARGV + 1) {
	print STDERR wrap("\nThe -t command line option requires a valid filesystem type as argument. See 'man mount' for filesystem types supported by your system.\n");
	usage();
      }
      $type_value = $ARGV[$i];
      next;
    }
    if ($ARGV[$i] eq "-o") {
      $i++;
      if ($i >= $#ARGV + 1) {
	print STDERR wrap("\nThe -o command line option requires at least one valid mount option as argument. See 'man mount' for options supported by your filesystem.\n");
	usage();
      }
      $options_value = $ARGV[$i];
      next;
    }
    push @arguments, $ARGV[$i];
  }
  
  # Check the options validity
  if (($print_mode == 1) && (not ($type_value eq ""))) {
    print STDERR wrap("\nOptions -p and -t are mutually exclusive.\n");
    usage();
  }
  if (($print_mode == 1) && (not ($options_value eq ""))) {
    print STDERR wrap("\nOptions -p and -o are mutually exclusive.\n");
    usage();
  }

  # Check the number of mandatory arguments
  if ($print_mode == 0) {
    $nb = 3;
  } else {
    $nb = 1;
  }
  if ($#arguments + 1 != $nb) {
    print STDERR wrap("\nThis script requires " . $nb . " (not " . ($#arguments + 1) . ") mandatory argument(s).\n");
    usage();
  }

  splash_screen();
  get_loop_binary();

  if ($print_mode == 0) {
    if (not is_root()) {
      print STDERR wrap("\nPlease re-run this script as the super user.\n");
      exit(1);
    }

    # Force the path to reduce the risk of using "modified" external helpers
    # If the user has a special system setup, he will be prompted for the
    # proper location anyway
    $ENV{'PATH'} = "/bin:/usr/bin:/sbin:/usr/sbin";

    initialize_external_helpers();

    if (Kernel_RunningVersion() < Kernel_MakeVersion(2, 1, 101)) {
      print STDERR wrap("\nYour Linux kernel is too old to run this script. Please boot a Linux kernel version 2.1.101 or higher (including 2.2.x) and re-run this script.\n");
      exit(1);
    }

    if (Kernel_RunningVersion() >= Kernel_MakeVersion(2, 4, 0)) {
      my $ans;

      $ans = query("It has been reported that this program does not work correctly with 2.4+ Linux kernels in some cases, and you are currently running such a kernel. Do you really want to continue?", "N");
      if ($ans !~ /^[yY]/) {
        print wrap("\nAborted.\n");
        exit(0);
      } 
    }

    export_mount($arguments[0], $arguments[1], $type_value, $arguments[2], $options_value);
  } else {
    show_partition_table($arguments[0]);
  }
  
  exit(0);
}

main();
