package org.v1al;
import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class FilterAdapter extends ArrayAdapter<String>{
	Context context;
	int current_filter = AHPSDRActivity.FILTER_5;
	
	public FilterAdapter (Context context, int resource, int textViewResourceId){
		super (context, resource, textViewResourceId);
		this.context = context;
	}
	
	public void setFilter(int filter){
		current_filter = filter;
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO Auto-generated method stub
		//return super.getView(position, convertView, parent);
		LayoutInflater inflater = ((Activity)context).getLayoutInflater();
		View row=inflater.inflate(R.layout.row, parent, false);
		TextView label=(TextView)row.findViewById(R.id.filter);
		label.setText(getItem(position));
		ImageView icon=(ImageView)row.findViewById(R.id.icon);
		if (position == current_filter) icon.setImageResource(R.drawable.ok);
		else icon.setImageResource(R.drawable.blue);
		return row;
	}
}
