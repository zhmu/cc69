#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

/*! \brief Runnable interface
 *
 *  This is used as an interface to the subapplications.
 */
class IRunnable {
public:
	//! \brief Virtual destructor to ensure all are called
	virtual ~IRunnable() { }

	//! \brief Initializes the object's data
	virtual void Init() = 0;

	//! \brief Cleans the object up
	virtual void Cleanup() = 0;

	//! \brief Starts the object
	virtual void Run() = 0;
};

#endif /*  __RUNNABLE_H__ */
