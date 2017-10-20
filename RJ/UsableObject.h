#pragma once

class iObject;


class UsableObject
{
public:
	
	// Default constructor
	UsableObject(void);

	// Event raised when a usable object is used; virtual method to be implemented by subclasses, does nothing if not implemented
	// Should return a flag indicating whether the object was 'successfully' used; by default, when this does nothing, the method returns false
	virtual bool OnUsed(iObject *user);


private:


};





