#pragma once

#include "Vector2.h"
#include "Vector.h"
#include <memory>
#include "Rect.h"
#include "RenderWindow.h"

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
private:
	//NOTE: All is public, but really you should only use the two methods, could make a constructor for that and therefore make it a class but yeah ;)
	struct PhysicElement
	{
		bool collidersInPointer;

		union
		{
			Collider* collidersPointer;
			Collider collidersValue;
		} colliders;
	public:
		const Collider* getCollider() const;
	};
public:
	class Body
	{
		friend class Physics;

		enum class TriggerBodyPart
		{
			NONE,
			HEAD,
			SHOES,
			LEFT,
			RIGHT
		};

		struct TriggerInformation
		{
			ShortString triggerElementCollision = "";
			TriggerBodyPart triggerBodyPart = TriggerBodyPart::NONE;
		};

		bool isStatic;
		bool isTrigger;
		bool triggered = false;
		bool isActive = true;
		TriggerInformation triggerInformation = {};
		Vector2f pos;
		ShortString id;
		Vector<PhysicElement> physicsElements;
		int index;
	public:
		Vector2f vel = { 0.0f, 0.0f };
	public:
		//Should be called, if the object is moving
		Body(Vector2f&& pos, const ShortString& name, Collider* collider, bool isTrigger = false, bool isStatic = false);
		//Should be called if the object, is a static one
		Body(const ShortString& name, Collider&& collider, bool isTrigger = false, bool isStatic = true);
		//To have one name for a lot of Colliders. The body you have to pass by value, because pos and that does not make sense to manipulate here!
		Body(const ShortString& name, Vector<Collider>&& colliders, bool isTrigger = false);
	public:
		bool getIsTriggerd() const;
		const Vector2f& getPos() const;
		void setPos(Vector2f newPos);
		const TriggerInformation& getTriggerInformation() const;
		const ShortString& getId() const;
		int getIndex() const;
		bool getIsActive() const;
		//TODO: TEST!!
		void flipActive(Physics& physics);
	};
private:
	static constexpr float gravity = 9.81f;
	size_t activeIndex = 0;
	size_t inactiveIndex = 0;
	Vector<Body> bodies;
private:
	void handleCollision(Body& itBody, Body& collideElementBody, const Collider & bodyCollider, const Collider& elementCollider);
public:
	Physics();
	void update(float dt);
	void debugRenderBodies(RenderWindow& window) const;
	//Use if you need a reference to the body, to get back triggerInformation etc.
	Body* addElementPointer(Body&& body);
	//Use this otherwise
	void addElementValue(Body&& body);
	void removeElementByIndex(int index);
};