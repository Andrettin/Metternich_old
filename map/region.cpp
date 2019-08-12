#include "map/region.h"

#include "map/province.h"
#include "util.h"

namespace Metternich {

QVariantList Region::GetProvincesQVariantList() const
{
	return ContainerToQVariantList(this->GetProvinces());
}

Q_INVOKABLE void Region::AddProvince(Province *province)
{
	this->Provinces.push_back(province);
	province->AddRegion(this);
}

Q_INVOKABLE void Region::RemoveProvince(Province *province)
{
	this->Provinces.erase(std::remove(this->Provinces.begin(), this->Provinces.end(), province), this->Provinces.end());
	province->RemoveRegion(this);
}

std::vector<Holding *> Region::GetHoldings() const
{
	std::vector<Holding *> holdings;

	for (const Province *province : this->GetProvinces()) {
		for (Holding *holding : province->GetHoldings()) {
			holdings.push_back(holding);
		}
	}

	return holdings;
}

}
