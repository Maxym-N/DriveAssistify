# Changelog

## Version 1.7
- Improvements: Disk area analysis now works reliably with exFAT and other filesystems not recognized by parted, by automatically detecting filesystem types using blkid when necessary.
- Usability: The "Show Disk Areas" function now displays a clear warning if the user attempts to run it on a partition instead of a whole disk, ensuring correct usage and preventing confusing output.
- Compatibility: All calculations involving sector size are now dynamic and use the actual sector size of the device, rather than assuming 512 bytes, improving support for modern storage devices (512e, 4Kn, etc.).
- Bug Fixes: Resolved an issue where exFAT partitions were incorrectly shown as "Free Space" in the disk area view.
- Code Quality: Improved robustness and maintainability by refactoring input validation and error handling in disk/partition operations.
- UI: Enhanced error and warning dialogs for unsupported operations and improved English-language messaging throughout the interface.
- Visuals:
  - Disk rows in the device list are now highlighted with a blue background and bold blue font, making it easier to distinguish disks from partitions.
  - Partition rows for each disk are now displayed in alphabetical order (e.g., sda1, sda2, sda5...), improving clarity and usability when working with complex storage layouts.

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
- UI: Improved disk/partition area display, including clear listing of free and used space, and more informative error dialogs.

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
