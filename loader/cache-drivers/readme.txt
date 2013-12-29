These drivers are all obsolete. They were used with an earlier version of propgcc where the cache tag handling was
in the driver. The new XMEM drivers handle tags in the XMM kernel.

*_cache.spin are cache drivers that keep their tags in COG memory
*_xcache.spin are cache drivers that keep their tags in hub memory
