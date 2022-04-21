#pragma once
#include "IMapDecorator.h"
#include <memory>
class IMapGenerator {
protected:
	std::shared_ptr<IMapDecorator> m_pMapDecorator;
public:
	IMapGenerator(std::shared_ptr<IMapDecorator> pMapDecorator) : m_pMapDecorator(pMapDecorator) {}
	virtual void generateGround() = 0;
	virtual void generateObjects() = 0;
	virtual void generateResources() = 0;
};