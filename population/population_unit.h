#pragma once

#include "population/population_unit_base.h"
#include "database/simple_data_type.h"

#include <set>

namespace metternich {

class culture;
class employment;
class holding;
class phenotype;
class population_type;
class province;
class religion;
class terrain_type;

class population_unit final : public population_unit_base, public simple_data_type<population_unit>
{
	Q_OBJECT

	Q_PROPERTY(metternich::population_type* type READ get_type WRITE set_type NOTIFY type_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(metternich::phenotype* phenotype READ get_phenotype WRITE set_phenotype NOTIFY phenotype_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding WRITE set_holding NOTIFY holding_changed)
	Q_PROPERTY(int wealth READ get_wealth NOTIFY wealth_changed)
	Q_PROPERTY(bool discount_any_type READ discounts_any_type WRITE set_discount_any_type NOTIFY discount_types_changed)
	Q_PROPERTY(QVariantList discount_types READ get_discount_types_qvariant_list NOTIFY discount_types_changed)

public:
	static constexpr const char *database_folder = "population_units";
	static constexpr int mixing_factor_permyriad = 1; //up to this fraction of people will "mix" per month per mixing check

	static void process_history_database();

	population_unit(population_type *type) : type(type)
	{
		connect(this, &population_unit::type_changed, this, &population_unit_base::icon_path_changed);
		connect(this, &population_unit::culture_changed, this, &population_unit_base::icon_path_changed);
		connect(this, &population_unit::religion_changed, this, &population_unit_base::icon_path_changed);
		connect(this, &population_unit::phenotype_changed, this, &population_unit_base::icon_path_changed);
	}

	virtual ~population_unit() override
	{
	}

	virtual void initialize_history() override;

	virtual void check_history() const override
	{
		if (this->get_culture() == nullptr) {
			throw std::runtime_error("Population unit has no culture.");
		}

		if (this->get_religion() == nullptr) {
			throw std::runtime_error("Population unit has no religion.");
		}

		if (this->get_phenotype() == nullptr) {
			throw std::runtime_error("Population unit has no phenotype.");
		}
	}

	void do_month();
	void do_mixing();
	void do_cultural_derivation();

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	population_type *get_type() const
	{
		return this->type;
	}

	void set_type(population_type *type)
	{
		if (type == this->get_type()) {
			return;
		}

		this->type = type;
		emit type_changed();
	}

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(religion *religion)
	{
		if (religion == this->get_religion()) {
			return;
		}

		this->religion = religion;
		emit religion_changed();
	}

	metternich::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	void set_phenotype(phenotype *phenotype)
	{
		if (phenotype == this->get_phenotype()) {
			return;
		}

		this->phenotype = phenotype;
		emit phenotype_changed();
	}

	void mix_with(population_unit *other_population_unit);

	virtual void set_size(const int size) override;

	metternich::holding *get_holding() const
	{
		return this->holding;
	}

	void set_holding(holding *holding);

	virtual void set_discount_existing(const bool discount_existing) override
	{
		if (discount_existing == this->discounts_existing()) {
			return;
		}

		if (discount_existing) {
			this->discount_types.insert(this->get_type()); //this population unit's type is implicitly added to the discount types if discount_existing is set to true
		} else {
			//if is being set to false, set the discount_any_type to false as well, and clear the discount types, as both are no longer applicable
			this->discount_types.clear();
		}

		population_unit_base::set_discount_existing(discount_existing);
	}

	bool discounts_any_type() const;
	void set_discount_any_type(const bool discount_any_type);

	const std::set<population_type *> &get_discount_types() const
	{
		return this->discount_types;
	}

	QVariantList get_discount_types_qvariant_list() const;

	Q_INVOKABLE void add_discount_type(population_type *type)
	{
		this->discount_types.insert(type);
		emit discount_types_changed();
	}

	Q_INVOKABLE void remove_discount_type(population_type *type)
	{
		this->discount_types.erase(type);
		emit discount_types_changed();
	}

	void subtract_existing_sizes();
	void subtract_existing_sizes_in_holding(const holding *holding);
	void subtract_existing_sizes_in_holdings(const std::vector<holding *> &holdings);
	bool can_distribute_to_holding(const holding *holding) const;
	void distribute_to_holdings(const std::vector<holding *> &holdings);

	int get_unemployed_size() const
	{
		return this->unemployed_size;
	}

	void set_unemployed_size(const int size)
	{
		if (size == this->get_unemployed_size()) {
			return;
		}

		this->unemployed_size = size;
	}

	void change_unemployed_size(const int change)
	{
		this->set_unemployed_size(this->get_unemployed_size() + change);
	}

	void add_employment(employment *employment)
	{
		this->employments.insert(employment);
	}

	void remove_employment(employment *employment)
	{
		this->employments.erase(employment);
	}

	void seek_employment();

	int get_wealth() const
	{
		return this->wealth;
	}

	void set_wealth(const int wealth)
	{
		if (wealth == this->get_wealth()) {
			return;
		}

		this->wealth = wealth;
		emit wealth_changed();
	}

	void change_wealth(const int change)
	{
		this->set_wealth(this->get_wealth() + change);
	}

	virtual const std::filesystem::path &get_icon_path() const override;

	const terrain_type *get_terrain() const;

signals:
	void type_changed();
	void culture_changed();
	void religion_changed();
	void phenotype_changed();
	void holding_changed();
	void terrain_changed();
	void discount_types_changed();
	void wealth_changed();

private:
	population_type *type = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	metternich::phenotype *phenotype = nullptr;
	metternich::holding *holding = nullptr; //the settlement holding where this population unit lives
	bool discount_any_type = false; //whether to discount the size of any existing population units from that of this one
	std::set<population_type *> discount_types; //the sizes of population units belonging to these types will be discounted from that of this population unit
	std::set<employment *> employments;
	int unemployed_size = 0; //the amount of people from this population unit which are unemployed
	int wealth = 0; //the wealth belonging to the population unit
};

}
