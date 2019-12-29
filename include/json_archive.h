#pragma once

#include <archive_common.h>
#include <QJsonDocument>
#include <QJsonArray>

struct variant_map_archive
{
	ARCHIVE
	QVariantMap * top_level;
};

QVariantMap * recursive_pup(QVariantMap * current, QStringList * list, QVariant * val, int io);

template<class T>
void pack_unpack(variant_map_archive & ar, QVector<T> & val_, const var_info & info_)
{
	QVariantMap * top = ar.top_level;
	
	// If outputting then build variant list from val_
	QVariantMap tl;
	QVariantList vl;

	if (ar.io == PUP_OUT)
	{
		ar.top_level = &tl;
		for (int i = 0; i < val_.size(); ++i)
		{
			QString key = "[" + QString::number(i) + "]";
			pack_unpack(ar, val_[i], var_info(key, QVariantMap()));
			vl.append(tl.value(key));
		}
		ar.top_level = top;
	}
	
	QStringList splt = info_.name.split(SPLIT_CHAR);
	QVariant val(vl);
	auto ret_vm = recursive_pup(ar.top_level, &splt, &val, ar.io);

	if (ar.io == PUP_IN && ret_vm == nullptr)
	{
		vl = val.toList();
		val_.resize(vl.size());
		tl.clear();
		ar.top_level = &tl;
		
		int i = 0;
		auto iter = vl.begin();
		while (iter != vl.end())
		{
			QString key = "[" + QString::number(i) + "]";
			tl[key] = *iter;
			pack_unpack(ar, val_[i], var_info(key, info_.params));
			++i;
			++iter;
		}
		ar.top_level = top;
	}
}

template<class T,class U>
void pack_unpack(variant_map_archive & ar, QPair<T,U> & val_, const var_info & info_)
{
	if (!info_.name.isEmpty())
		pack_unpack(ar, val_.first, var_info(info_.name + QString(SPLIT_CHAR) + QString("first"), info_.params));
	else
		pack_unpack(ar, val_.first, var_info("first", info_.params));
	if (!info_.name.isEmpty())
		pack_unpack(ar, val_.second, var_info(info_.name + QString(SPLIT_CHAR) + QString("second"), info_.params));
	else
		pack_unpack(ar, val_.second, var_info("second", info_.params));
}

template<class Val>
void pack_unpack(variant_map_archive & ar, QMap<QString, Val> & val_, const var_info & info_)
{
	QVariantMap * top = ar.top_level;	
	QVariantMap tl;

	if (ar.io == PUP_OUT)
	{
		ar.top_level = &tl;
		auto iter = val_.begin();
		while (iter != val_.end())
		{
			pack_unpack(ar, iter.value(), var_info(iter.key(), QVariantMap()));
			++iter;
		}
		ar.top_level = top;
	}
	
	QStringList splt = info_.name.split(SPLIT_CHAR);
	QVariant val(tl);
	auto ret_vm = recursive_pup(ar.top_level, &splt, &val, ar.io);

	if (ar.io == PUP_IN && ret_vm == nullptr)
	{
		tl = val.toMap();
		ar.top_level = &tl;
		auto iter = tl.begin();
		while (iter != tl.end())
		{
			Val v;
			pack_unpack(ar, v, var_info(iter.key(), QVariantMap()));
			val_[iter.key()] = v;
			++iter;
		}
		ar.top_level = top;
	}
}

template<class Val>
void pack_unpack(variant_map_archive & ar, QMap<int, Val> & val_, const var_info & info_)
{
	QVariantMap * top = ar.top_level;	
	QVariantMap tl;

	if (ar.io == PUP_OUT)
	{
		ar.top_level = &tl;
		auto iter = val_.begin();
		while (iter != val_.end())
		{
			pack_unpack(ar, iter.value(), var_info(QString::number(iter.key()), QVariantMap()));
			++iter;
		}
		ar.top_level = top;
	}
	
	QStringList splt = info_.name.split(SPLIT_CHAR);
	QVariant val(tl);
	auto ret_vm = recursive_pup(ar.top_level, &splt, &val, ar.io);

	if (ar.io == PUP_IN && ret_vm == nullptr)
	{
		tl = val.toMap();
		ar.top_level = &tl;
		auto iter = tl.begin();
		while (iter != tl.end())
		{
			Val v;
			pack_unpack(ar, v, var_info(iter.key(), QVariantMap()));
			val_[iter.key().toInt()] = v;
			++iter;
		}
		ar.top_level = top;
	}
}

void pack_unpack(variant_map_archive & ja, QVariant & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, QString & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, char & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, wchar_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, int8_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, int16_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, int32_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, int64_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, uint8_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, uint16_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, uint32_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, uint64_t & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, float & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, double & val_, const var_info & info_);
void pack_unpack(variant_map_archive & ja, bool & val_, const var_info & info_);

void pack_unpack(variant_map_archive & ar, QMap<QString, QVariant> & val_, const var_info & info_);

template<class T>
void write_ptr_vec_to_json_doc(QVector<T*> & vec, QJsonDocument & doc)
{
	QVariantList vl;
	variant_map_archive ar;
	ar.io = PUP_OUT;

	for(int i = 0; i < vec.size(); ++i)
	{
		QVariantMap vm;
		ar.top_level = &vm;
		pack_unpack(ar, *vec[i], var_info("", QVariantMap()));
		vl.append(vm);
	}
	doc.setArray(QJsonArray::fromVariantList(vl));
}

template<class T>
void read_json_doc_to_ptr_vec(QVector<T*> & vec, QJsonDocument & doc)
{
	QVariantList vl = doc.toVariant().toList();
	variant_map_archive ar;
	ar.io = PUP_IN;

	vec.resize(vl.size());
	auto iter = vl.begin();
	int i = 0;
	while(iter != vl.end())
	{
		QVariantMap vm = iter->toMap();
		ar.top_level = &vm;
			
		vec[i] = new T;
		pack_unpack(ar, *vec[i], var_info("", QVariantMap()));
		++iter;
		++i;
	}
}