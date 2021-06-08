#ifndef SHAREABLEDATA_H
#define SHAREABLEDATA_H

#include "data-storage-abstraction_global.h"

#include <type_traits>

#include <QString>
#include <QVarLengthArray>

#include "datastorage.h"


/*
 * thin manager of I/O to/from T of type derived from DataStorage.
 *
 * generalises handling of data with the following relation:
 * unordered group of unordered groups of data where each datum is identifiable
 * as a manifestation of a certain specification inside the group
 * it is directly grouped in and
 * the specifications inside that group are and the number of them is the same
 * as inside every other group across all groups inside the group that group is
 * directly grouped in.
 *
 * the outer group is called "collection" here.
 *
 * the value of the number of inner groups the collection groups
 * is an integer equal or larger than 0.
 *
 * inner groups can be added and can be removed.
 *
 * a specification of a datum inside an inner group here consists of
 * a "designator" labelling the datum and of a type.
 *
 * that type is inferred from the declaration of the data.
 *
 * the storage of the data passed in and out to an instance of this class
 * is shared across all instances of this class except expressively
 * wished otherwise.
 *
 * example of data this class handles:
 * data inside rows of a table, the table head constitutes the specifications.
 *
 * D == number of designators, equalling number of datum per inner group,
 * to get added to use in appropriate method,
 *
 * I == number of data to get added to optimise for,
 *
 * O == number of data to get returned to optimise for
 */

template<class T, qsizetype D = 32, qsizetype I = 4, qsizetype O = 16>

class DATASTORAGEABSTRACTION_EXPORT ShareableData
{
	static_assert(std::is_base_of<DataStorage, T>::value, "ShareableData requires the class inside <> is derived from DataStorage");

public:
	enum class CollectionState {
		/*
		 * collection identified by identifier in setCollectionIdentifier
		 * did not exist yet,
		 * asPrivate == false || asPrivate == true,
		 * asUnique == false || asUnique == true
		 */
		Created_New,
		/*
		 * collection identified by identifier in setCollectionIdentifier
		 * did exist,
		 * asUnique == true,
		 * asPrivate == false || asPrivate == true,
		 * another instance must use the CollectionIdentifier representation
		 * of identifier if it wants to access the data (if asPrivate == false)
		 */
		Created_Iuxta,
		/*
		 * collection identified by identifier in setCollectionIdentifier
		 * did exist,
		 * asUnique == false,
		 * asPrivate == false,
		 * another instance can use the equal QString representation of
		 * identifier you passed in
		 */
		Affiliated,
		/*
		 * collection identified by identifier in setCollectionIdentifier
		 * did exist,
		 * asUnique == false,
		 * asPrivate == true or
		 * this instance already identifies a collection or
		 * an error occurred preventing one of the other states (to be known)
		 */
		Aborted
	};

	typedef int Identifier;

	typedef Identifier CollectionIdentifier;
	typedef Identifier DesignatorIdentifier;
	typedef Identifier DataIdentifier;

	/*
	 * pass asPrivate = true if you do want NOT to let other instances access
	 * the collection identified by the identifier set in
	 * setCollectionIdentifier.
	 * notwithstanding the collection will be stored in a shared location.
	 * use inPersonalDataStorage = true to change this.
	 *
	 * pass asUnique = true if you want for certain to have a dedicated
	 * collection not yet existing if the identifier set in
	 * setCollectionIdentifier identifies an already existing collection.
	 *
	 * pass inPersonalDataStorage = true to expressively state you wish data
	 * passed in here to not be stored in a shared location.
	 *
	 * setCollectionIdentifier is a separate method because it returns,
	 * a ShareableData::CollectionState to let you know whether this instance
	 * can access that collection.
	 */
	ShareableData(bool asPrivate = false, bool asUnique = false, bool inPersonalDataStorage = false);
	~ShareableData();

	/*
	 * returns true when created asPrivate, else false
	 */
	bool isPrivate();

	/*
	 * returns true when created asUnique, else false
	 */
	bool isUnique();

	/*
	 * returns true when created inPersonalDataStorage, else false
	 */
	bool isInPersonalDataStorage();

	/*
	 * set an identifier for the collection which is let created here if it
	 * does not exist yet or of the collection you want to access.
	 *
	 * this method must be called for the instance to be usable.
	 *
	 * mind the returned value!
	 *
	 * an instance can only identify 1 collection.
	 * if you call this method after it already successfully identifies
	 * a collection, ShareableDate::CollectionState::Aborted is returned.
	 */
	ShareableData::CollectionState setCollectionIdentifier(QString identifier);
	ShareableData::CollectionState setCollectionIdentifier(ShareableData::CollectionIdentifier identifier);

	/*
	 * return the identifier identifying this collection represented as
	 * CollectionIdentifier
	 *
	 * use isValidIdentifier to know whether the identifier is valid,
	 * it is invalid for example as long as it has been not set yet.
	 *
	 * note: no benefit is I being aware of in returning the QString
	 * representation because that is either already known to the creator,
	 * is slower to use for identification from others
	 * and might be a privacy issue for the creator when known to others.
	 */
	ShareableData::CollectionIdentifier getCollectionIdentifier();

	bool isValidIdentifier(ShareableData::Identifier identifier) {
		return identifier >= 0;
	}

	/*
	 * designators are accepted only once. each of them must be unique within
	 * the QVarLengthArray they come in.
	 *
	 * returns an empty QVarLengthArray if some other designators
	 * already present.
	 */
	QVarLengthArray<ShareableData::DesignatorIdentifier, D> addDesignators(QVarLengthArray<QString, D> &designators);

	/*
	 * returns an empty QVarLengthArray if designators not added yet.
	 */
	QVarLengthArray<ShareableData::DesignatorIdentifier, D> getDesignators();

	/*
	 * each QVarLengthArray is to have the same length.
	 * they contain data per designator.
	 * across all of these are inner group.
	 *
	 * returns an empty QVarLengthArray if data didn't get added
	 * (for example when designators don't exist).
	 *
	 * returned QVarLengthArray's length otherwise equals data's
	 * QVarLengthArray length.
	 *
	 * returned QVarLengthArray's entries each identify an inner group
	 * across all designators.
	 * the value of the position each of them is in is the value of the position
	 * the data each of them identfies across QVarLengthArray is in in the
	 * QVarLengthArray in data.
	 */
	QVarLengthArray<ShareableData::DataIdentifier, I> addData(QHash<ShareableData::DesignatorIdentifier, QVarLengthArray<QVariant, I>> &data);
	QVarLengthArray<ShareableData::DataIdentifier, I> addData(QHash<QString, QVarLengthArray<QVariant, I>> &data);

	/*
	 * returns collection, each datum in the inner QVarLengthArray manifests the
	 * specification designated by the identifier given at the position
	 * in identifiers having the same value as the position of the datum.
	 *
	 * returns an empty QVarLengthArray if identifiers didn't match a single
	 * present designator.
	 */
	QVarLengthArray<QVarLengthArray<QVariant, D>, O> getDataOf(QVarLengthArray<ShareableData::DesignatorIdentifier, D> identifiers);
	/*
	 * returns data per designator. the value of the position the data across
	 * all of those is in has is the value of the position the identifier
	 * identifying that data has in identifiers.
	 *
	 * returns an empty QHash if identifiers didn't match a single present inner
	 * group.
	 */
	QHash<ShareableData::DesignatorIdentifier, QVarLengthArray<QVariant, O>> getDataBy(QVarLengthArray<ShareableData::DataIdentifier, D> identifiers);


private:
	static T storage;
	T* personalStorage;
	bool asPrivate;
	bool asUnique;
	bool inPersonalDataStorage;
	CollectionIdentifier theManagedCollection = -1;
	QVarLengthArray<DesignatorIdentifier, D> designators;
};

#endif // SHAREABLEDATA_H
