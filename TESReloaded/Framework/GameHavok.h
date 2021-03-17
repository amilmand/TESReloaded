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

class bhkPackedNiTriStripsShape : public bhkRefObject {
public:
	UInt32			unk010;		// 010
	UInt32			unk014;		// 014
};
assert(sizeof(bhkPackedNiTriStripsShape) == 0x18);

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
	UInt32		unk010;			// 010
	UInt32		unk014;			// 014
	UInt32		unk018;			// 018
	UInt32		unk01C;			// 01C
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
	hkPackedNiTriStripsData*	PackedNiTriStripsData;	// 010
};
assert(sizeof(hkPackedNiTriStripsShape) == 0x14);

class hkRigidBody : public hkRefObject {
public:
	hkWorld*			World;			// 008
	bhkRigidBody*		bRigidBody;		// 00C
	UInt32				unk010;			// 010
	hkShape*			Shape;			// 014 always a hkMoppBvTreeShape?
	UInt32				unk018;
	hkBaseObject*		unk01C;
	UInt32				unk020;
	UInt32				unk024;
	UInt32				unk028;
	UInt32				unk02C;
	UInt32				unk030;
	UInt32				unk034;
	UInt32				unk038;
	UInt32				unk03C;
	UInt32				unk040;
	UInt32				unk044;
	UInt32				unk048;
	UInt32				unk04C;
	hkBaseObject*		unk050;
	UInt32				unk054;
	UInt8				unk058;			// 058
	UInt8				pad058[3];
	UInt32				unk05C;			// 05C
	UInt32				unk060;			// 060
	UInt32				unk064;
	UInt32				unk068;
	UInt32				unk06C;
	UInt32				unk070;
	UInt32				unk074;
	UInt32				unk078;
	UInt32				unk07C;
	UInt32				unk080;
	UInt32				unk084;
	UInt32				unk088;
	UInt16				unk08C;
	UInt16				unk08E;			// 08E
	UInt32				unk090;
};
assert(sizeof(hkRigidBody) == 0x94);