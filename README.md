# OS Libraries
Current libraries:
- bitmap (v1.5)
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

- dyn_array (v1.3)
	- It's a vector, it's a stack, it's a deque, it's all your hopes and dreams!
	- Supports destructors! Function pointers are fun.
	- Wishlist:
		- Better insert_sorted (bsearch-based)
		- shrink_to_fit (add a flag to the struct, have it be read by dyn_request_size_increase)
		- prune (remove those who match a certain criteria (via function pointer))
		- Rename export to data (that's what C++ calls it)???
			- Change export to return a copy and not live data(?)
		- Inserting/removing n objects at a time
		    - Dyn's core already supports this, the API doesn't.
		    	- I didn't want to write more unit tests...
		- Rewrite bool functions to int(?)

Coming "soon":

- dyn_list
	- It's a list, it stores things!
	- Doubly-linked, circular, fancy.
	- Actually called clist now!

Eventually (maybe):

- dyn_hash
	- It's a hash table, it stores AND hashes things! Exciting!

- dyn_string
	- dyn_array, but for managing strings!

-- Will, the best TA
