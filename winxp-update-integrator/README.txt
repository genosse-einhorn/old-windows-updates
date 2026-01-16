windows update integrator
=========================

or: fuck me did I just reimplement HFSLIP?

requirements:

* Linux distro with usual GNU userland (tested with Debian)
* mkisofs
* cabextract
* gcab
* exiftool

short howto:

* copy original files into folder orig-iso
* copy updates to slipstream into update-repo
* copy other updates (if any) into addons
* create kblist.txt, either manually or via ./mkkblist.sh > kblist.txt
* extract updates with ./extract-updates.sh
* slipstream them via ./do-integration.sh
* create ISO image with ./mkiso.sh

todo:

* replace files based on version instead of modification date
* figure out additional hacks to directly integrate more updates
* figure out how to deal with IE8 and WMP11
