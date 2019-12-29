#ifndef PUPPER_H
#define PUPPER_H

#define PUP_OUT 1
#define PUP_IN 2

const char SPLIT_CHAR = 26;

#include <QColor>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QTimeZone>
#include <QVector>
#include <QDataStream>


// When new pup types are added make sure to add the types here
#define PUP_BASE_CLASS \
	virtual void pack_unpack(variant_map_archive & ar, const var_info & info)=0;

#define PUP_DERIVED_CLASS_DECL 											\
	void pack_unpack(variant_map_archive & ar, const var_info & info);

#define PUP_DERIVED_CLASS_DEF(class_type) \
	void class_type::pack_unpack(variant_map_archive & ar, const var_info & info) \
	{ \
		::pack_unpack(ar, *this, info);			\
	}


#define ARCHIVE int16_t io; \
	QString name;

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#define pup_func(type) \
	template<class ArchiveT> \
	void pack_unpack(ArchiveT & ar, type & val, const var_info & info)

#define pup_func_templated_class(class_type) \
	template<class ArchiveT, class T>											\
	void pack_unpack(ArchiveT & ar, class_type<T> & val, const var_info & info)


#define pup_friend(class_type) \
	template<class ArchiveT> \
	friend void pack_unpack(ArchiveT & ar, class_type & val, const var_info & info)

#define pup_friend_templated_class(class_type)					\
	template<class ArchiveT, class T>											\
	friend void pack_unpack(ArchiveT & ar, class_type<T> & val, const var_info & info)

#define pup_member(member_name) \
	if (!info.name.isEmpty())	\
		pack_unpack(ar, val.member_name, var_info(info.name + QString(SPLIT_CHAR) + QString(STR(member_name)), info.params)); \
	else \
		pack_unpack(ar, val.member_name, var_info(STR(member_name), info.params))

#define pup_member_dispname(member_name,disp_name) \
	if (!info.name.isEmpty())	\
		pack_unpack(ar, val.member_name, var_info(info.name + QString(SPLIT_CHAR) + QString(disp_name), info.params)); \
	else \
		pack_unpack(ar, val.member_name, var_info(disp_name, info.params))


#define pup_base(base_type) \
	pack_unpack(ar, static_cast<base_type &>(val), info)

#define pup_enum(enum_type, member_name) \
	int32_t tmp = static_cast<int32_t>(val.member_name); \
	pack_unpack(ar, tmp, var_info(info.name + QString(SPLIT_CHAR) + QString(STR(member_name)), info.params)); \
	val.member_name = static_cast<enum_type>(tmp)

struct var_info
{
	var_info(const QString & var_name, const QVariantMap & params_):
		name(var_name),
		params(params_)
	{}

	virtual ~var_info() {}

	QString name;
	QVariantMap params;
};

template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QDate & val_, const var_info & info_)
{
	QString dt_str = val_.toString();
	pack_unpack(ar, dt_str, info_);
	val_ = QDate::fromString(dt_str);
}

template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QByteArray & val_, const var_info & info_)
{
	QVector<int8_t> byte_vec;
	byte_vec.resize(val_.size());
	for (int i = 0; i < val_.size(); ++i)
		byte_vec[i] = val_[i];

	pack_unpack(ar, byte_vec, info_);

	val_.resize(byte_vec.size());
	for (int i = 0; i < byte_vec.size(); ++i)
		val_[i] = byte_vec[i];
}

template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QTimeZone & val_, const var_info & info_)
{
	QByteArray id = val_.id();
	pack_unpack(ar, id, info_);
	val_ = QTimeZone(id);
}

template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QTime & val_, const var_info & info_)
{
	QString dt_str = val_.toString();
	pack_unpack(ar, dt_str, info_);
	val_ = QTime::fromString(dt_str);
}

template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QColor & val_, const var_info & info_)
{
	int r = val_.red();
	int g = val_.green();
	int b = val_.blue();
	int a = val_.alpha();
	pack_unpack(ar, r, var_info(info_.name + QString(SPLIT_CHAR) + "r", info_.params));
	pack_unpack(ar, g, var_info(info_.name + QString(SPLIT_CHAR) + "g", info_.params));
	pack_unpack(ar, b, var_info(info_.name + QString(SPLIT_CHAR) + "b", info_.params));
	pack_unpack(ar, a, var_info(info_.name + QString(SPLIT_CHAR) + "a", info_.params));
	val_ = QColor(r,g,b,a);
}


template<class ArchiveT>
void pack_unpack(ArchiveT & ar, QDateTime & val_, const var_info & info_)
{
	QString dt_str = val_.toString();
	pack_unpack(ar, dt_str, info_);
	val_ = QDateTime::fromString(dt_str);
}

template<class ArchiveT, class T>
void pack_unpack(ArchiveT & ar, QVector<T> & val_, const var_info & info_)
{
	int sz = val_.size();
	pack_unpack(ar, sz, var_info(info_.name + QString(SPLIT_CHAR) + "size", QVariantMap()));
	val_.resize(sz);
	for (int i = 0; i < sz; ++i)
	{
		QString name = info_.name + "[" + QString::number(i) + "]";
		pack_unpack(ar, val_[i], var_info(name, info_.params));
	}
}

template<class ArchiveT, class T>
void pack_unpack(ArchiveT & ar, QSet<T> & val_, const var_info & info_)
{
	int sz = val_.size();
	pack_unpack(ar, sz, var_info(info_.name + QString(SPLIT_CHAR) + "size", QVariantMap()));

	auto iter = val_.begin();
	for (int i = 0; i < sz; ++i)
	{
		T val;
		
		if (ar.io == PUP_OUT)
		{
			val = *iter;
			++iter;
		}
		
		QString name = info_.name + "[" + QString::number(i) + "]";
		pack_unpack(ar, val, var_info(name, info_.params));
		val_.insert(val);
	}
}

template<class ArchiveT, class Key, class Val>
void pack_unpack(ArchiveT & ar, QMap<Key, Val> & val_, const var_info & info_)
{
	int sz = val_.size();
	pack_unpack(ar, sz, var_info(info_.name + QString(SPLIT_CHAR) + "size", QVariantMap()));

	auto iter = val_.begin();
    for (int i = 0; i < sz; ++i)
	{
		Key k; Val v;

		if (ar.io == PUP_OUT)
		{
			k = iter.key();
			v = iter.value();
			++iter;
		}
		
		QString item_name = info_.name + "[" + QString::number(i) + "]";
		
		pack_unpack(ar, k, var_info(item_name + QString(SPLIT_CHAR) + "key", info_.params));
		pack_unpack(ar, v, var_info(item_name + QString(SPLIT_CHAR) + "value", info_.params));
		val_[k] = v;
	}
}

#endif
