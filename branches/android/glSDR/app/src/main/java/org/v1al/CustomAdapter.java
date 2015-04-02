package org.v1al;
import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class CustomAdapter extends ArrayAdapter<String>{
	Context context;
	int current_selection = 0;
	
	public CustomAdapter (Context context, int resource, int textViewResourceId){
		super (context, resource, textViewResourceId);
		this.context = context;
	}
	
	public void setSelection(int selection){
		current_selection = selection;
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO Auto-generated method stub
		//return super.getView(position, convertView, parent);
		LayoutInflater inflater = ((Activity)context).getLayoutInflater();
		View row=inflater.inflate(R.layout.row, parent, false);
		TextView label=(TextView)row.findViewById(R.id.selection);
		label.setText(getItem(position));
		ImageView icon=(ImageView)row.findViewById(R.id.icon);
		if (position == current_selection) icon.setImageResource(R.drawable.ok);
		else icon.setImageResource(R.drawable.blue);
		return row;
	}
}
