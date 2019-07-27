#pragma once

#include <QObject>
#include <QString>

#include <memory>
#include <mutex>

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class Metternich : public QObject
{
	Q_OBJECT
	
public:
	static Metternich *GetInstance();
	
private:
	static inline std::unique_ptr<Metternich> Instance;
	static inline std::once_flag OnceFlag;

public:
	Q_INVOKABLE QString GetAssetImportPath() const;
};
