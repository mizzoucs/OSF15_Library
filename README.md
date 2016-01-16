# OS Library Graveyard
- block_store (v2.0.2)
	- Generic in-memory block storage system with optional file linking
	- Wishlist:
		- Better testing. Flush testing is a little lighter than I'd like, but I'm tired of looking at it
		- Better utility implementation, block based. Actually, may be better to just offload block work to the OS? (the way it is now)
		- Make in-memory optional, add flag (FILE_BACKED) and detect and switch internal functions if it's set
			- mmap? anonymous files? Voodoo?

Libraries required for building:

- bitmap (1.5)
	- It's a bitmap, it stores bits!
	- Wishlist:
		- FLZ/FLS
		- a for_each for ALL bits, which passes the bit # and a bool (???)
			- Rename current to for_each_set
			- Which just begs the question of for_each_unset
				- HMMMMMM...
		- Better FFZ/FFS/for_each
		- Resumeable FFS/FFZ/FLZ/FLS ?
		- Parameter checking
			- Just never give us a bad pointer or bit address and it's fine :p
		- Rename export to data (that's what C++ calls it)???


-- Will, the best TA
