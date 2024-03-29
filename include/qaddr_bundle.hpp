#pragma once

#include "block/qinputs.hpp"
#include "block/qoutputs.hpp"
#include "block/qunlocks.hpp"
#include "client/qnode_outputs.hpp"

#if defined(USE_QML)
#include<QtQml>
#endif

#include <QObject>
#include <QtCore/QtGlobal>
#if defined(WINDOWS_QADDR)
# define QADDR_EXPORT Q_DECL_EXPORT
#else
#define QADDR_EXPORT Q_DECL_IMPORT
#endif

namespace qiota{

using namespace qblocks;

#if defined(USE_QML)
class QADDR_EXPORT Qml64:public QObject
{
    quint64 m_value;
    QJsonObject m_json;
    Q_OBJECT

    Q_PROPERTY(QJsonObject  json READ json  NOTIFY jsonChanged)
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    Qml64(const quint64 value=0,QObject *parent = nullptr);
    QJsonObject json(void)const;
    void setValue(const quint64 value);
    quint64 getValue()const;
signals:
    void jsonChanged();
private:
    void intToJSON();
};

class QADDR_EXPORT AddressChecker:public QObject
{
    bool  m_valid;
    QString m_address;
    void setValid(bool valid);
    Q_OBJECT

    Q_PROPERTY(bool valid READ isValid  NOTIFY validChanged)
    Q_PROPERTY(QString address MEMBER m_address   NOTIFY addressChanged)
    QML_ELEMENT

public:
    AddressChecker(QObject *parent = nullptr);
    bool isValid(void)const;
signals:
    void validChanged();
    void addressChanged();
};
#endif
struct InBox {
    std::shared_ptr<const Input> input=nullptr;
    std::shared_ptr<const Output> retOutput=nullptr;
    qblocks::c_array inputHash;
    std::shared_ptr<Output> output=nullptr;
    quint64 amount=0;
};
class QADDR_EXPORT AddressBox:public QObject
{
    Q_OBJECT
#if defined(USE_QML)
    Q_PROPERTY(Qml64*  amount READ amountJson CONSTANT)
    Q_PROPERTY(QString bech32Address READ getAddressBech32 CONSTANT)
    QML_ELEMENT
    QML_UNCREATABLE("")
#endif

public:
    AddressBox(const std::pair<QByteArray,QByteArray>& keyPair,const QString hrp="rms",QObject *parent = nullptr);

    AddressBox(const std::shared_ptr<const Address>& addr,c_array outId=c_array(),const QString hrp="rms",QObject *parent = nullptr);

    std::shared_ptr<const Address> getAddress(void)const;
    QString getAddressHash(void)const;
    QString getAddressBech32()const;

    void getOutputs(std::vector<Node_output> &outs, const quint64 amountNeedIt=0, const quint16 howMany=0);
    pvector<const Unlock> getUnlocks
        (const QByteArray & message, const quint16 &ref, const size_t &inputSize)const;
    quint64 amount(void)const;
#if defined(USE_QML)
    Qml64* amountJson()const;
#endif
    const QHash<c_array,InBox>& inputs(void)const;
    c_array outId()const;

signals:
    void amountChanged(quint64 prevA,quint64 nextA);
    void changed();
    void inputRemoved(std::vector<c_array>);
    void inputAdded(c_array);
    void addrAdded(AddressBox * );
    void addrRemoved(std::vector<c_array>);

private:
    void clean(std::vector<c_array> &rm_inputs, std::vector<c_array> &rm_addresses);
    void monitorToSpend(const c_array outId);
    void monitorToExpire(const c_array outId, const quint32 unixTime);
    void monitorFromExpire(const c_array outId, const quint32 unixTime);
    void monitorToUnlock(const c_array outId,const quint32 unixTime);
    void rmInput(const c_array outId,std::vector<c_array> & rm_inputs,std::vector<c_array> & rm_addresses);
    void addInput(const c_array outId, const InBox &inBox);
    void addAddrBox(const c_array outId, AddressBox* addrBox);

    void setAmount(const quint64 amount);

    QHash<c_array,InBox> m_inputs;
    QHash<c_array,AddressBox *> m_AddrBoxes;
    quint64 m_amount;
#if defined(USE_QML)
    Qml64* m_amountJson;
#endif
    const std::pair<QByteArray,QByteArray> m_keyPair;
    const std::shared_ptr<const Address> m_addr;
    const c_array m_outId;
    const QString m_bech32address,m_hrp;

};
};
