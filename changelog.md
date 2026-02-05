# Changelog

## Version 1.8
- Features: Added full GRUB installation support for BIOS/MBR and UEFI systems, with separate functions for each mode.
- Features: Added automatic disk list refresh after all disk operations complete, eliminating the need for manual refresh.
- Features: Added support for quick format and full zeroing options for NTFS filesystems in both partition creation and formatting dialogs.
- Improvements: Enhanced partition creation with an optional alignment setting that allows users to align partitions to 1 MiB boundaries (2048 sectors) for optimal compatibility with modern storage devices.
- Improvements: Enhanced partition creation for NVMe and GPT disks with dual partition detection mechanism (lsblk + parted fallback) for maximum reliability.
- Improvements: Terminal output now shows step-by-step progress for GRUB installation (mounting, grub-install, unmounting) and reports errors clearly.
- Improvements: Partition rename dialog now displays the current label and pre-fills it in the input field, making it easy to edit existing labels.
- Improvements: Enhanced FAT32/vfat label support with automatic fallback between fatlabel, dosfslabel, mlabel, and blkid tools for maximum compatibility.
- Improvements: All partition labeling operations now properly use sudo to ensure sufficient permissions.
- Improvements: Partition creation now uses sector-based commands (parted mkpart primary [start]s [end]s) for precise control and to avoid parted's decimal/binary unit confusion.
- Improvements: Free space calculation during partition creation and resize operations now displays correctly in real-time as users adjust size values.
- Improvements: Mount and unmount operations now properly display "Command Finished" status in terminal window title when operations complete.
- Improvements: All mount and unmount commands now use sudo for proper privilege elevation, ensuring reliable execution.
- Improvements: Added udevadm settle synchronization after filesystem creation for improved stability on NVMe and SSD devices.
- Bug Fixes: Fixed partition deletion destroying entire GPT partition table on SSDs by replacing fdisk with parted for GPT-compatible partition removal.
- Bug Fixes: Fixed partition creation failures on NVMe drives and GPT disks where newly created partitions were not detected, causing "No such file or directory" errors during filesystem creation.
- Bug Fixes: Fixed free space display showing "0 MiB" during partition creation by properly utilizing orig_free variable instead of recalculating from start/end positions.
- Bug Fixes: Fixed all label_free calculations in on_entry_gib_changed and on_entry_sectors_changed functions to use correct base values.
- Bug Fixes: Partition resize now works correctly.
- Bug Fixes: Fixed hangs caused by stale mount points or unresponsive devices during GRUB installation.
- Bug Fixes: Fixed terminal window not showing completion status for mount operations by updating the on_mount_child_exited callback to properly set window title.
- Code Quality: Refactored GRUB logic into dedicated MBR and UEFI functions, improved error handling, quoting, and cleanup for temporary directories and mounts.
- Code Quality: Added proper forward declarations for static callback functions (on_entry_gib_changed, on_entry_sectors_changed, on_entry_start_changed, on_entry_end_changed).
- Code Quality: Implemented numerous additional optimizations, bug fixes, and code improvements throughout the application.
- Compatibility: Partition alignment is now optional and configurable via checkbox, with 1 MiB alignment recommended but not enforced.
- Compatibility: Partition deletion now automatically detects partition table type (GPT vs MBR) and uses appropriate tool (parted for GPT, fdisk for MBR).
- Compatibility: Enhanced compatibility with modern NVMe SSDs through improved partition detection and udev synchronization.
- UI/UX: Rename partition dialog now shows partition name, filesystem type, and current label, with the label pre-selected for easy editing.
- UI/UX: Added "Align partition (start at 1 MiB boundary, recommended)" checkbox in partition creation dialog for user control over alignment behavior.
- UI/UX: NTFS formatting now offers two distinct options: "ntfs (quick format)" for fast formatting and "ntfs (full zeroing)" for secure data erasure.

## Version 1.7
- Improvements: Disk area analysis now works reliably with exFAT and other filesystems not recognized by parted, by automatically detecting filesystem types using blkid when necessary.
- Usability: The "Show Disk Areas" function now displays a clear warning if the user attempts to run it on a partition instead of a whole disk, ensuring correct usage and preventing confusing output.
- Compatibility: All calculations involving sector size are now dynamic and use the actual sector size of the device, rather than assuming 512 bytes, improving support for modern storage devices (512e, 4Kn, etc.).
- Bug Fixes: Resolved an issue where exFAT partitions were incorrectly shown as "Free Space" in the disk area view.
- Code Quality: Improved robustness and maintainability by refactoring input validation and error handling in disk/partition operations.
- UI/UX: Enhanced error and warning dialogs for unsupported operations and improved English-language messaging throughout the interface.
- UI/UX: Disk rows in the device list are now highlighted with a blue background and bold blue font, making it easier to distinguish disks from partitions.
- UI/UX: Partition rows for each disk are now displayed in alphabetical order (e.g., sda1, sda2, sda5...), improving clarity and usability when working with complex storage layouts.

## Version 1.6
- Features: Added secure multi-pass erase (shred) for partitions and filesystems, with confirmation dialogs and progress output.
- Features: Added partition deletion functionality with automatic unmount and kernel table refresh.
- Features: Added interactive management of "Free Space" areas—users can now create new partitions and filesystems directly in unallocated space via a graphical dialog.
- Features: Added creation of new partition tables (MBR/GPT) with automatic detection and confirmation.
- Features: Added full workflow for new partition creation and formatting, including filesystem type selection (ext2/3/4, NTFS, exFAT, FAT32, etc.).
- Improvements: After destructive or structural operations (shred, delete, create partition/table, format), the program now automatically re-reads the partition table using partprobe or, if unavailable, falls back to blockdev --rereadpt for maximum compatibility.
- Usability: The terminal window now changes its title to "Command Finished" when the operation completes, providing clear feedback to the user about command execution status.
- Features: Added partition rename functionality supporting ext2/3/4, NTFS, exFAT, FAT32, XFS, and Btrfs filesystems.
- Features: Added a "Toggle Boot Flag" menu item. If Parted is available, the boot flag is set or removed automatically; if not, user instructions for Fdisk are shown.
- Improvements: GRUB installation now uses a safer temporary mount point and displays clearer warnings. Boot flag operations are separated into a dedicated function.
- Code Quality: Refactored code to improve modularity and maintainability, including the separation of boot flag logic and enhanced signal handling for terminal operations.
- UI/UX: Improved disk/partition area display, including clear listing of free and used space, and more informative error dialogs.

## Version 1.5
- Security & Safety: Added confirmation dialogs (with clear English warnings) before all potentially destructive operations, including partition erasure, image restore, GRUB installation, and multi-pass erase.
- Usability: Improved user guidance by displaying detailed instructions and device names before running critical commands.
- Reliability: Enhanced device/partition detection logic for NVMe and other device types to avoid accidental operations on the wrong target.
- Other: Minor UI refinements and code cleanup.

## Version 1.4
- New Features: Added NTFS filesystem repair and resize options, along with a detailed disk scan feature.  
- Improvements: Enhanced error handling, faster disk list loading, and improved context menu layout.  
- Bug Fixes: Resolved issues with partition display and forced unmount failures.  
- Other: Updated license information and performed minor code cleanup for stability.  
