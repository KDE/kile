/**********************************************************************

	--- Qt Architect generated file ---

	File: legendOp.h
	Last generated: Sun Feb 22 19:45:48 1998

 *********************************************************************/

#ifndef legendOp_included
#define legendOp_included

#include "legendOpData.h"
#include "gnuInterface.h"

class legendOp : public legendOpData
{
    Q_OBJECT

public:

    legendOp
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~legendOp();

  void setGnuInterface(gnuInterface* gnu);
  void setLegendOpDefaults();
  void setLegendOptions();

private:
  gnuInterface* gnuInt;

};
#endif // legendOp_included
