#ifndef MeshStage_HPP__
#define MeshStage_HPP__

#include "AbstractStage.hpp"
#include "BlockingQueue.h"
#include <boost/any.hpp>

using namespace std;

class MeshStage : public AbstractStage
{
public:

	// default constructor
	MeshStage();

	virtual void firstStep();
	virtual void step();
	virtual void lastStep();
	
private:
	size_t mesh_count_;
};
#endif // STAGE
