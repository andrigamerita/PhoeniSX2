/*  AetherSX2 - PS2 Emulator for Android and ARM PCs
 *  Copyright (C) 2022 AetherSX2 Dev Team
 *
 *  AetherSX2 is provided under the terms of the Creative Commons
 *  Attribution-NonCommercial-NoDerivatives International License
 *  (BY-NC-ND 4.0, https://creativecommons.org/licenses/by-nc-nd/4.0/).
 * 
 *  Commercialization of this application and source code is forbidden.
 */

#pragma once

#include <array>

// nVifBlock - Ordered for Hashing; the 'num' and 'upkType' fields are
//             used as the hash bucket selector.
union nVifBlock {
	// Warning: order depends on the newVifDynaRec code
	struct {
		u8 num;			// [00] Num Field
		u8 upkType; 	// [01] Unpack Type [usn1:mask1:upk*4]
		u16 length; 	// [02] Extra: pre computed Length
		u32 mask;		// [04] Mask Field
		u8 mode;		// [08] Mode Field
		u8 aligned; 	// [09] Packet Alignment
		u8 cl;			// [10] CL Field
		u8 wl;			// [11] WL Field
		uptr startPtr;	// [12] Start Ptr of RecGen Code
	};

	struct {
		u16 hash_key;
		u16 _pad0;
		u32 key0;
		u32 key1;
		uptr value;
	};

}; // 16 bytes

// 0x4000 is enough but 0x10000 allow
// * to skip the compare value of the first double world in lookup
// * to use a 16 bits move instead of an 'and' mask to compute the hashed key
#define hSize 0x10000 // [usn*1:mask*1:upk*4:num*8] hash...

// HashBucket is a container which uses a built-in hash function
// to perform quick searches. It is designed around the nVifBlock structure
//
// The hash function is determined by taking the first bytes of data and
// performing a modulus the size of hSize. So the most diverse-data should
// be in the first bytes of the struct. (hence why nVifBlock is specifically sorted)
class HashBucket {
protected:
	std::array<nVifBlock*, hSize> m_bucket;

public:
	HashBucket() {
		m_bucket.fill(nullptr);
	}

	~HashBucket() { clear(); }

	__fi nVifBlock* find(const nVifBlock& dataPtr) {
		nVifBlock* chainpos = m_bucket[dataPtr.hash_key];

		while (true) {
			if (chainpos->key0 == dataPtr.key0 && chainpos->key1 == dataPtr.key1)
				return chainpos;

			if (chainpos->startPtr == 0)
				return nullptr;

			chainpos++;
		}
	}

	void add(const nVifBlock& dataPtr) {
		u32 b = dataPtr.hash_key;

		u32 size = bucket_size( dataPtr );

		// Warning there is an extra +1 due to the empty cell
		// Performance note: 64B align to reduce cache miss penalty in `find`
#if !defined(__APPLE__) || !defined(_M_ARM64)
		if( (m_bucket[b] = (nVifBlock*)pcsx2_aligned_realloc( m_bucket[b], sizeof(nVifBlock)*(size+2), 64, sizeof(nVifBlock)*(size+1) )) == NULL )
#else
		if( (m_bucket[b] = (nVifBlock*)realloc( m_bucket[b], sizeof(nVifBlock)*(size+2) )) == NULL )
#endif
		{
			throw Exception::OutOfMemory(
				wxsFormat(L"HashBucket Chain (bucket size=%d)", size+2)
			);
		}

		// Replace the empty cell by the new block and create a new empty cell
		memcpy(&m_bucket[b][size++], &dataPtr, sizeof(nVifBlock));
		memset(&m_bucket[b][size], 0, sizeof(nVifBlock));

		if( size > 3 ) DevCon.Warning( "recVifUnpk: Bucket 0x%04x has %d micro-programs", b, size );
	}

	u32 bucket_size(const nVifBlock& dataPtr) {
		nVifBlock* chainpos = m_bucket[dataPtr.hash_key];

		u32 size = 0;

		while (chainpos->startPtr != 0) {
			size++;
			chainpos++;
		}

		return size;
	}

	void clear() {
		for (auto& bucket : m_bucket)
		{
#if !defined(__APPLE__) || !defined(_M_ARM64)
			safe_aligned_free(bucket);
#else
			free(bucket);
#endif
		}
	}

	void reset() {
		clear();

		// Allocate an empty cell for all buckets
		for (auto& bucket : m_bucket) {
#if !defined(__APPLE__) || !defined(_M_ARM64)
			if( (bucket = (nVifBlock*)_aligned_malloc( sizeof(nVifBlock), 64 )) == nullptr )
#else
			if( (bucket = (nVifBlock*)malloc( sizeof(nVifBlock))) == nullptr )
#endif
			{
				throw Exception::OutOfMemory(
						wxsFormat(L"HashBucket Chain (bucket size=%d)", 1)
						);
			}

			memset(bucket, 0, sizeof(nVifBlock));
		}
	}
};