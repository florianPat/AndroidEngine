#pragma once

#include "Vector2.h"
#include "Vector.h"
#include "Rect.h"
#include "Window.h"

class Physics
{
public:
	struct FloatCircle
	{
		float radius;
		Vector2f center;
	public:
		FloatCircle(Vector2f&& center, float radius);
		FloatCircle(float centerX, float centerY, float radius);
		FloatCircle() = default;
	};

	class OBB
	{
	private:
		friend class Physics;

		float angle;
		Vector2f xAxis, yAxis;
	public:
		float width, height;
		Vector2f pos;
		Vector2f origin;
	public:
		//angle has to be in degrees!
		OBB(float left, float top, float width, float height, float angle);
		OBB(Vector2f&& topLeft, float width, float height, float angle);
		//Local origin
		OBB(float left, float top, float width, float height, float angle, Vector2f&& origin);
		OBB(Vector2f&& topLeft, float width, float height, float angle, Vector2f&& origin);
		//angle has to be in degrees!
		void setAngle(float newAngle);
		float getAngle() const;
		OBB() = default;
	};

	class Collider
	{
	public:
		enum class Type
		{
			rect,
			obb,
			circle
		};
	private:
		friend class Physics;

		Type type;
	public:
		union
		{
			FloatRect rect;
			OBB obb;
			FloatCircle circle;
		} collider;

		Collider(FloatRect& rect);
		Collider(OBB& obb);
		Collider(FloatCircle& circle);

		Collider(FloatRect&& rect);
		Collider(OBB&& obb);
		Collider(FloatCircle&& circle);

		Type GetType() const;
	private:
		Collider();

		bool intersects(const Collider& other) const;
		bool collide(const Collider& other, Vector2f* minTransVec) const;

		void getPointsAxis(Vector2f* points, Vector2f* axis) const;
		Vector2f getProjectionMinMax(const Vector2f* points, const Vector2f& axis, bool isXAxis) const;
	};
public:
	class Body
	{
		friend class Physics;
	public:
		enum class TriggerBodyPart
		{
			NONE,
			HEAD,
			SHOES,
			LEFT,
			RIGHT
		};
	private:
		struct TriggerInformation
		{
			ShortString triggerElementCollision = "";
			int index = -1;
			TriggerBodyPart triggerBodyPart = TriggerBodyPart::NONE;
		};

		bool isStatic;
		bool isTrigger;
		bool triggered = false;
		bool isActive = true;
    private:
		TriggerInformation triggerInformation = {};
		ShortString id;
		Vector<Collider> physicsElements;
		Vector<int> collisionLayers;
	public:
		Vector2f pos;
		Vector2f vel = { 0.0f, 0.0f };
	public:
		//Should be called, if the object is moving
		Body(Vector2f&& pos, const ShortString& name, Collider&& collider, Vector<int>&& collideLayers,
				bool isTrigger = false, bool isStatic = false);
		//Should be called if the object, is a static one
		Body(const ShortString& name, Collider&& collider, bool isTrigger = false, bool isStatic = true);
		//To have one name for a lot of Colliders. The body you have to pass by value, because pos and that does not make sense to manipulate here!
		Body(const ShortString& name, Vector<Collider>&& colliders, bool isTrigger = false);
	public:
		bool getIsTriggerd();
		const TriggerInformation& getTriggerInformation() const;
		const ShortString& getId() const;
		Collider& getCollider();
		void setIsActive(bool isActive);
		bool getIsActive() const;
	private:
		void checkCollideLayers();
	};
public:
	static constexpr float GRAVITY = -9.81f;
private:
	static constexpr int NUM_LAYERS = 4;
	Vector<Body> bodies;
	int collisionLayers[NUM_LAYERS];
	//TODO: Can I get rid of this?
	Vector<int> bodyIndices;
private:
	void handleCollision(Body& itBody, Body& collideElementBody, const Collider & bodyCollider, const Collider& elementCollider, int bodyIndex);
public:
	Physics();
	void update(float dt);
	void debugRenderBodies(Graphics& gfx) const;
	//Use if you need a reference to the body, to get back triggerInformation etc.
	int addElementPointer(Body&& body, int layer);
	//You need to call this each frame, because the Body could be "moved" to another memory location
	int getRealIndex(int index) const;
	Body* getBodyFromRealIndex(int realIndex);
	//Use this otherwise
	void addElementValue(Body&& body, int layer);
	Vector<ShortString> getAllCollisionIdsWhichContain(const ShortString & string);
};