#include <json_archive.h>
#include <QJsonObject>

QVariantMap * recursive_pup(QVariantMap * current, QStringList * list, QVariant * val, int pup_mode)
{
	if (list->size() == 1)
	{
        if (pup_mode == PUP_IN)
        {
            auto fiter = current->find(list->back());
            if (fiter != current->end())
                *val = *fiter;
            else
                return current;
        }
        else
        {
		    (*current)[list->back()] = *val;
        }   
		return nullptr;
	}
	else
	{
        if (pup_mode == PUP_IN)
        {
            auto fiter = current->find(list->front());
            if (fiter != current->end())
                current = static_cast<QVariantMap*>(fiter->data());
            else
                return current;
        }
        else
        {
            QVariant & vm = (*current)[list->front()];
            if (!vm.isValid())
            {
                vm.setValue(QVariantMap());
                current = static_cast<QVariantMap*>(vm.data());
            }
            else
            {
                current = static_cast<QVariantMap*>(vm.data());
            }
        }
		list->pop_front();
		return recursive_pup(current, list, val, pup_mode);
	}
}

void pack_unpack(variant_map_archive & ar, QString & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toString();
}

void pack_unpack(variant_map_archive & ar, char & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toChar().toLatin1();
}

void pack_unpack(variant_map_archive & ar, wchar_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toChar().toLatin1();	
}

void pack_unpack(variant_map_archive & ar, int8_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<int8_t>(val.toInt());
}

void pack_unpack(variant_map_archive & ar, int16_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<int16_t>(val.toInt());
}

void pack_unpack(variant_map_archive & ar, int32_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<int32_t>(val.toInt());
}

void pack_unpack(variant_map_archive & ar, int64_t & val_, const var_info & info_)
{
	QVariant val(static_cast<qlonglong>(val_));
	pack_unpack(ar, val, info_);
	val_ = static_cast<int64_t>(val.toLongLong());
}

void pack_unpack(variant_map_archive & ar, uint8_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<uint8_t>(val.toUInt());
}

void pack_unpack(variant_map_archive & ar, uint16_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<uint16_t>(val.toUInt());
}

void pack_unpack(variant_map_archive & ar, uint32_t & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = static_cast<uint32_t>(val.toUInt());
}

void pack_unpack(variant_map_archive & ar, uint64_t & val_, const var_info & info_)
{
	QVariant val(static_cast<qulonglong>(val_));
	pack_unpack(ar, val, info_);
	val_ = static_cast<uint64_t>(val.toULongLong());
}

void pack_unpack(variant_map_archive & ar, float & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toFloat();
}

void pack_unpack(variant_map_archive & ar, QVariant & val_, const var_info & info_)
{
	QStringList splt = info_.name.split(SPLIT_CHAR);
	recursive_pup(ar.top_level, &splt, &val_, ar.io);
}

void pack_unpack(variant_map_archive & ar, double & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toDouble();
}

void pack_unpack(variant_map_archive & ar, bool & val_, const var_info & info_)
{
	QVariant val(val_);
	pack_unpack(ar, val, info_);
	val_ = val.toBool();
}

void pack_unpack(variant_map_archive & ar, QMap<QString, QVariant> & val_, const var_info & info_)
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
			QVariant v;
			pack_unpack(ar, v, var_info(iter.key(), QVariantMap()));
			if (v.isValid())
				val_[iter.key()] = v;
			else
				val_.remove(iter.key());
			++iter;
		}
		ar.top_level = top;
	}
}
