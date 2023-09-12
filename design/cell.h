#pragma once

#include "common.h"
#include "formula.h"

#include <set>

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
private:
    /*
        Ѕазовый класс Impl дл€ €чеек разных типов и три наследника: 
            пуста€ €чейка EmptyImpl, 
            формульна€ €чейка FormulaImpl,
            текстова€ €чейка TextImpl.
    */
    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const override;

        virtual bool IsCache();
        virtual void InvalidateCache();

        ~Impl() = default;

    };
    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text, SheetInterface& sheet_);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        // -- ћетоды кэш €чейки -- //
        bool IsCache() override;
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_impl_;
        mutable std::optional<FormulaInterface::Value> cache_;      //кэш-значение в формульной €чейки, optional и хран. в FormulaImpl из вебинара
        SheetInterface& sheet_;
    };

    std::unique_ptr<Impl> impl_;

    std::set<Cell*> dependent_cells_;   // вычисл€емые €чейки
    std::set<Cell*> link_cells_;        // €чейки св€зующие 

};