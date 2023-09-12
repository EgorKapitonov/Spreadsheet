#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(Sheet& sheet) 
	: impl_(std::make_unique<EmptyImpl>())
	, sheet_(sheet) {
}

Cell::~Cell() = default;

/*
	Когда пользователь задаёт текст в методе Cell::Set(), 
	внутри метода определяется тип ячейки в зависимости от заданного текста 
	и создаётся нужный объект-реализация: 
		формульная, 
		текстовая, 
		пустая.
*/
void Cell::Set(std::string text) {

	std::unique_ptr<Impl> impl_ptr;
	// если пустая
	if (text.empty()) {
		impl_ptr = std::make_unique<EmptyImpl>();
	}
	// если ячейка формульная
	else if (text.at(0) == FORMULA_SIGN) {
		impl_ptr = std::make_unique<FormulaImpl>(std::move(text), sheet_);
	}
	// если текстовая
	else {
		impl_ptr = std::make_unique<TextImpl>(std::move(text));
	}

	// Поиск циклических зависимостей 
	const Impl& impl_temp = *impl_ptr;
	const auto temp_ref_cells = impl_temp.GetReferencedCells();

	if (!temp_ref_cells.empty()) {

		std::set<const Cell*> ref_collection;
		std::set<const Cell*> enter_collection;
		std::vector<const Cell*> to_enter_collection;

		for (auto pos : temp_ref_cells) {
			ref_collection.insert(sheet_.Get_Cell(pos));
		}

		to_enter_collection.push_back(this);
		while (!to_enter_collection.empty()) {
			const Cell* ongoing = to_enter_collection.back();

			to_enter_collection.pop_back();
			enter_collection.insert(ongoing);

			if (ref_collection.find(ongoing) == ref_collection.end()) {

				for (const Cell* dependent : ongoing->dependent_cells_) {

					if (enter_collection.find(dependent) == enter_collection.end()) {
						to_enter_collection.push_back(dependent);
					}
				}

			}
			else {
				throw CircularDependencyException("circular dependency detected");
			}
		}
		impl_ = std::move(impl_ptr);
	}
	else {
		impl_ = std::move(impl_ptr);
	}
	// Конец поиска

	// Обновление зависимостей
	for (Cell* refrenced : referenced_cells_) {
		refrenced->dependent_cells_.erase(this);
	}

	referenced_cells_.clear();

	for (auto& pos : impl_->GetReferencedCells()) {

		Cell* refrenced = sheet_.Get_Cell(pos);

		if (!refrenced) {
			sheet_.SetCell(pos, "");
			refrenced = sheet_.Get_Cell(pos);
		}

		referenced_cells_.insert(refrenced);
		refrenced->dependent_cells_.insert(this);
	}
	// Конец обновление зависимостей
	InvalidateAllCache(true);
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_ ->GetReferencedCells();
}

bool Cell::IsReferenced() const {
	return !dependent_cells_.empty();
}
void Cell::InvalidateAllCache(bool flag = false) {
	if (impl_->IsCache() || flag) {
		impl_->InvalidateCache();

		for (Cell* dependent : dependent_cells_) {
			dependent->InvalidateAllCache();
		}
	}
}

// --- Определение методов класса Impl --- //
std::vector<Position> Cell::Impl::GetReferencedCells() const {
	return {};
}

bool Cell::Impl::IsCache() {
	return true;
}
void Cell::Impl::InvalidateCache() {
}

// --- Определение методов подкласса EmptyImpl --- //
Cell::Value Cell::EmptyImpl::GetValue() const {
	return "";
}
std::string Cell::EmptyImpl::GetText() const {
	return "";
}

// --- Определение методов подкласса TextImpl --- //
Cell::TextImpl::TextImpl(std::string text)
	: text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
	if (text_.empty()) {
		throw std::logic_error("empty TextImpl");
	}
	else if (text_.at(0) == ESCAPE_SIGN) {
		return text_.substr(1);
	}
	else {
		return text_;
	}
}
std::string Cell::TextImpl::GetText() const {
	return text_;
}

// --- Определение методов подкласса FormulaImpl --- //
Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet)
	: formula_impl_(ParseFormula(text.substr(1)))
	, sheet_(sheet) {
}

Cell::Value Cell::FormulaImpl::GetValue() const {

	if (!cache_) {
		cache_ = formula_impl_->Evaluate(sheet_);
	}
	return std::visit([](auto& helper) {
		return Value(helper); 
		}, *cache_);
}

std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_impl_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_impl_->GetReferencedCells();
}

bool Cell::FormulaImpl::IsCache() {
	return cache_.has_value();
}
void Cell::FormulaImpl::InvalidateCache() {
	cache_.reset();
}