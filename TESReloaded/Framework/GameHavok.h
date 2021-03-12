#pragma once

class hkMoppCode;
class hkPackedNiTriStripsShape;

class bhkCharacterProxy;

class hkVector4 {
public:
	float	x;
	float	y;
	float	z;
	float	w;
};
assert(sizeof(hkVector4) == 0x010);

class hkBaseObject {
public:
	virtual void Destructor(bool FreeThis);
};
assert(sizeof(hkBaseObject) == 0x004);

class hkRefObject : public hkBaseObject {
public:
	UInt16		sizeAndFlags;	// 04
	UInt16		refCount;		// 06
};
assert(sizeof(hkRefObject) == 0x008);

class bhkRefObject : public NiObject {
public:
	hkRefObject*		hkObject;	// 008
	void*				hkData;		// 00C
};
assert(sizeof(bhkRefObject) == 0x010);

class bhkRigidBody : public bhkRefObject {
public:
	UInt32			unk010;		// 010
	UInt32			unk014;		// 014
};
assert(sizeof(bhkRigidBody) == 0x18);

class NiCollisionObject : public NiObject {
public:
	NiAVObject*		SceneObject;	// 008
};
assert(sizeof(NiCollisionObject) == 0x0C);

class hkPackedNiTriStripsData : public NiObject {
public:
	UInt32		NumTriangles;	// 008
	UInt32		NumVertexes;	// 00C
	NiObject*	Shape1;			// 010
	NiObject*	Shape2;			// 014
	NiObject*	Shape3;			// 018
	NiObject*	Shape4;			// 01C
};
assert(sizeof(hkPackedNiTriStripsData) == 0x20);

class bhkCollisionObject : public NiCollisionObject {
public:
	enum {
		kFlag_Active	= 1 << 0,
		kFlag_Reset		= 1 << 1,
		kFlag_Notify	= 1 << 2,
		kFlag_SetLocal	= 1 << 3,
	};

	UInt16				flags;			// 00C
	UInt8				pad00E[2];		// 00E
	bhkRigidBody*		bRigidBody;		// 010
};
assert(sizeof(bhkCollisionObject) == 0x14);

class hkWorld : public hkRefObject {
public:
	void*		simulation;				// 008
	float		unk00C;					// 00C
#if defined(OBLIVION) || defined(SKYRIM)
	hkVector4	unk020;					// 010
#endif
	hkVector4	gravity;				// 020
	// More...
};
assert(sizeof(hkWorld) == 0x030);

class hkShape : public hkRefObject {
public:
	bhkRefObject*				bRefObject;				// 008
	hkPackedNiTriStripsShape*	PackedNiTriStripsShape;	// 00C
};
assert(sizeof(hkShape) == 0x10);

class hkPackedNiTriStripsShape : public hkShape {
public:
	hkPackedNiTriStripsData*	PackedNiTriStripsData;		// 010
};
assert(sizeof(hkPackedNiTriStripsShape) == 0x14);

class hkMoppBvTreeShape : public hkShape {
public:
	hkMoppCode*					MoppCode;				// 010
};
assert(sizeof(hkMoppBvTreeShape) == 0x14);

class hkRigidBody : public hkRefObject {
public:
	hkWorld*			World;				// 008
	bhkRigidBody*		bRigidBody;			// 00C
	UInt32				unk010;				// 010
	hkMoppBvTreeShape*	Shape;				// 014 always a hkMoppBvTreeShape?
	// More...
};
assert(sizeof(hkRigidBody) == 0x18);